**********************
Nginx Thread Pool Note
**********************

Nginx uses thread pool to schedule tasks, which has several advantages:

    * pre-create fixed number worker threads to avoid frequently creating and destroying threads for each task
    * avoid frequent context switching

However, these advantages also come with costs, including:

    * more complex thread synchronization logic
    * may need to take consideration of thread contention, schedule strategy etc.

Nginx thread pool implementation can be found:

    - ngx_thread_pool.h, ngx_thread_pool.c

Some structure definition and interface declarations are posted here:

.. code-block:: c

    struct ngx_thread_task_s
    {
        ngx_thread_task_t   *next;
        ngx_uint_t           id;
        void                *ctx;
        void               (*handler)(void *data, ngx_log_t *log);
        ngx_event_t          event;
    };

    struct ngx_thread_pool_s
    {
        ngx_thread_mutex_t        mtx;
        ngx_thread_cond_t         cond;

        // task queue implemented using singly linked list
        ngx_thread_pool_queue_t   queue;

        // task count threshold, default to 65536
        ngx_int_t                 max_queue;

        // current task count in pool, may be negative
        ngx_int_t                 waiting;

        // worker thread count, default to 32
        ngx_uint_t                threads;

        ngx_log_t                *log;
        ngx_str_t                 name;

        u_char                   *file;
        ngx_uint_t                line;
    };

    typedef struct ngx_thread_pool_s  ngx_thread_pool_t;

    // add one task to thread pool
    ngx_int_t ngx_thread_task_post(ngx_thread_pool_t *tp, ngx_thread_task_t *task);

Nginx reads the configuration, pre-creates several worker threads.
after proper initialization, workers are waitting to prcess the tasks inserted into
the thread pool's task queue using ``ngx_thread_task_post``, when a task is processed
by a worker, then it is inserted into the finished task queue ``ngx_thread_pool_done``,
and some post-process will be done in ``ngx_notify``.

Following are code snippets showing the calling hierarchy:

.. code-block:: c

    static ngx_int_t ngx_thread_pool_init(ngx_thread_pool_t *tp, ngx_log_t *log, ngx_pool_t *pool)
    {
        // ...
        pthread_t       tid;
        for (ngx_uint_t　n = 0; n < tp->threads; n++)
        {
            err = pthread_create(&tid, &attr, ngx_thread_pool_cycle, tp);
            if (err) {
                ngx_log_error(NGX_LOG_ALERT, log, err,
                              "pthread_create() failed");
                return NGX_ERROR;
            }
        }
        // ...
    }

    static void* ngx_thread_pool_cycle(void *data)
    {
        // ...
        for ( ;; )
        {
            /* the number may become negative */
            tp->waiting--;

            // pop a task from thread pool queue
            ngx_thread_task_t* task = tp->queue.first;
            tp->queue.first = task->next;

            if (tp->queue.first == NULL) {
                tp->queue.last = &tp->queue.first;
            }

            // task-specific handler set by callers
            task->handler(task->ctx, tp->log);
            task->next = NULL;

            ngx_spinlock(&ngx_thread_pool_done_lock, 1, 2048);

            // push the finished task to finished task queue
            *ngx_thread_pool_done.last = task;
            ngx_thread_pool_done.last = &task->next;

            ngx_memory_barrier();

            ngx_unlock(&ngx_thread_pool_done_lock);

            // TODO: add notes about post-process
            (void) ngx_notify(ngx_thread_pool_handler);
        }
    }

    static void ngx_thread_pool_handler(ngx_event_t *ev)
    {
        ngx_spinlock(&ngx_thread_pool_done_lock, 1, 2048);

        ngx_thread_task_t* task = ngx_thread_pool_done.first;
        ngx_thread_pool_done.first = NULL;
        ngx_thread_pool_done.last = &ngx_thread_pool_done.first;

        ngx_memory_barrier();

        ngx_unlock(&ngx_thread_pool_done_lock);

        // drain tasks in finished queue
        while (task)
        {
            ngx_event_t* event = &task->event;
            task = task->next;

            event->complete = 1;
            event->active = 0;

            event->handler(event);
        }
    }

    ngx_int_t ngx_thread_task_post(ngx_thread_pool_t *tp, ngx_thread_task_t *task)
    {
        // ...
        if (ngx_thread_mutex_lock(&tp->mtx, tp->log) != NGX_OK) {
            return NGX_ERROR;
        }

        if (tp->waiting >= tp->max_queue)
        {
            (void) ngx_thread_mutex_unlock(&tp->mtx, tp->log);

            ngx_log_error(NGX_LOG_ERR, tp->log, 0,
                          "thread pool \"%V\" queue overflow: %i tasks waiting",
                          &tp->name, tp->waiting);
            return NGX_ERROR;
        }

        task->id = ngx_thread_pool_task_id++;

        if (ngx_thread_cond_signal(&tp->cond, tp->log) != NGX_OK)
        {
            (void) ngx_thread_mutex_unlock(&tp->mtx, tp->log);
            return NGX_ERROR;
        }

        // add one task to the task queue
        // build a singly linked list
        *tp->queue.last = task;
        tp->queue.last = &task->next;

        tp->waiting++;

        (void) ngx_thread_mutex_unlock(&tp->mtx, tp->log);

        // ...
    }

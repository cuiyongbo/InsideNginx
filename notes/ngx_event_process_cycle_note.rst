******************************
Nginx Event Process Cycle Note
******************************

In nginx, event process cycle is mainly about signal processing, here
I post it to clarify the calling hierarchy of event cycle.
Source files:

    - ngx_cycle.h, ngx_cycle.c
    - ngx_process_cycle.h, ngx_process_cycle.c

you can see it in following code sinppt:

.. code-block:: c

    int ngx_cdecl main(int argc, char *const *argv)
    {
        // ...
        if (ngx_process == NGX_PROCESS_SINGLE)
            ngx_single_process_cycle(cycle);
        else
            ngx_master_process_cycle(cycle);
    }

    void ngx_master_process_cycle(ngx_cycle_t *cycle)
    {
        // ...
        ngx_start_worker_processes(cycle, ccf->worker_processes, NGX_PROCESS_RESPAWN);
    }

    static void ngx_start_worker_processes(ngx_cycle_t *cycle, ngx_int_t n, ngx_int_t type)
    {
        for (ngx_int_t i = 0; i < n; i++)
        {
            ngx_spawn_process(cycle, ngx_worker_process_cycle,
                         (void *)(intptr_t)i, "worker process", type);
           // ...
        }
    }

    ngx_pid_t ngx_spawn_process(ngx_cycle_t *cycle, ngx_spawn_proc_pt proc, void *data, char *name, ngx_int_t respawn)
    {
        // ...
        ngx_pid_t pid = fork();
        switch (pid)
        {
        case -1:
            ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_errno,
                          "fork() failed while spawning \"%s\"", name);
            ngx_close_channel(ngx_processes[s].channel, cycle->log);
            return NGX_INVALID_PID;
        case 0:
            ngx_parent = ngx_pid;
            ngx_pid = ngx_getpid();
            proc(cycle, data);
            break;
        default:
            break;
        }
    }

    static void ngx_worker_process_cycle(ngx_cycle_t *cycle, void *data)
    {
        ngx_int_t worker = (intptr_t) data;
        ngx_process = NGX_PROCESS_WORKER;
        ngx_worker = worker;
        ngx_worker_process_init(cycle, worker);
        ngx_setproctitle("worker process");

        for ( ;; )
        {
            if (ngx_exiting)
            {
                if (ngx_event_no_timers_left() == NGX_OK)
                {
                    ngx_log_error(NGX_LOG_NOTICE, cycle->log, 0, "exiting");
                    ngx_worker_process_exit(cycle); // call exit()
                }
            }

            // the workhorse
            ngx_process_events_and_timers(cycle);

            if (ngx_terminate)
            {
                ngx_log_error(NGX_LOG_NOTICE, cycle->log, 0, "exiting");
                ngx_worker_process_exit(cycle);
            }

            if (ngx_quit)
            {
                ngx_quit = 0;
                ngx_log_error(NGX_LOG_NOTICE, cycle->log, 0, "gracefully shutting down");
                ngx_setproctitle("worker process is shutting down");

                if (!ngx_exiting)
                {
                    ngx_exiting = 1;
                    ngx_set_shutdown_timer(cycle);
                    ngx_close_listening_sockets(cycle);
                    ngx_close_idle_connections(cycle);
                }
            }

            if (ngx_reopen)
            {
                ngx_reopen = 0;
                ngx_log_error(NGX_LOG_NOTICE, cycle->log, 0, "reopening logs");
                ngx_reopen_files(cycle, -1);
            }
        }
    }

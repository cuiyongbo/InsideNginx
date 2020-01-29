****************
Nginx Event Note
****************

Nginx event contains intialization and cleanup stuffs, event processing interfaces (the workhorse of nginx).
Here I focuse on ``ngx_process_events_and_timers`` function, which is about how nginx schedules event
connections. Its implementation can be found

    - ngx_event.h, ngx_event.c

I post the code snippt in the following section:

.. code-block:: c

    void ngx_process_events_and_timers(ngx_cycle_t *cycle)
    {
        // ...
        {
            // the next event to process
            timer = ngx_event_find_timer();
            flags = NGX_UPDATE_TIME;
        }

        // ...

        ngx_msec_t delta = ngx_current_msec;

        // the true workhorse
        ngx_process_events(cycle, timer, flags);

        delta = ngx_current_msec - delta;

        ngx_event_process_posted(cycle, &ngx_posted_accept_events);

        if (delta)
        {
            // maybe events expired during processing current event
            ngx_event_expire_timers();
        }

        ngx_event_process_posted(cycle, &ngx_posted_events);
    }

Some macro methods are also posted for reviewing:

.. code-block:: c

    extern ngx_event_actions_t   ngx_event_actions;

    #define ngx_process_events   ngx_event_actions.process_events
    #define ngx_done_events      ngx_event_actions.done

    #define ngx_add_event        ngx_event_actions.add
    #define ngx_del_event        ngx_event_actions.del
    #define ngx_add_conn         ngx_event_actions.add_conn
    #define ngx_del_conn         ngx_event_actions.del_conn

    #define ngx_notify           ngx_event_actions.notify

    #define ngx_add_timer        ngx_event_add_timer
    #define ngx_del_timer        ngx_event_del_timer

    // ``ngx_event_actions`` are properly initialized in configured module,
    // in linux if nginx event configuration is unspecified,
    // it is initialized using epoll module.
    static ngx_int_t ngx_epoll_init(ngx_cycle_t *cycle, ngx_msec_t timer)
    {
        // ...
        ngx_event_actions = ngx_epoll_module_ctx.actions;
    }

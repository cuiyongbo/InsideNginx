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
        ngx_uint_t  flags;
        ngx_msec_t  timer;
        if (ngx_timer_resolution)
        {
            timer = NGX_TIMER_INFINITE;
            flags = 0;
        }
        else
        {
            timer = ngx_event_find_timer();
            flags = NGX_UPDATE_TIME;
        }

        if (ngx_use_accept_mutex)
        {
            if (ngx_accept_disabled > 0)
            {
                ngx_accept_disabled--;
            }
            else
            {
                if (ngx_trylock_accept_mutex(cycle) == NGX_ERROR)
                    return;

                if (ngx_accept_mutex_held)
                {
                    flags |= NGX_POST_EVENTS;
                }
                else
                {
                    if (timer == NGX_TIMER_INFINITE || timer > ngx_accept_mutex_delay)
                        timer = ngx_accept_mutex_delay;
                }
            }
        }

        ngx_msec_t delta = ngx_current_msec;

        // the true workhorse
        ngx_process_events(cycle, timer, flags);

        delta = ngx_current_msec - delta;

        ngx_event_process_posted(cycle, &ngx_posted_accept_events);

        if (ngx_accept_mutex_held)
        {
            ngx_shmtx_unlock(&ngx_accept_mutex);
        }

        if (delta)
        {
            ngx_event_expire_timers();
        }

        ngx_event_process_posted(cycle, &ngx_posted_events);
    }

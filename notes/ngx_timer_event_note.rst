**********************
Nginx Timer Event Note
**********************

Nginx timer event is implemented using the red black tree introduced in :doc:`ngx_red_black_tree_note`.
and supports add, delete, search operations. Its implementation and application can be found in

    - ngx_event_timer.h, ngx_event_timer.c
    - ngx_event.h, ngx_event.c

    .. code-block:: c

        ngx_int_t ngx_event_timer_init(ngx_log_t *log);

        ngx_msec_t ngx_event_find_timer(void);
        void ngx_event_del_timer(ngx_event_t *ev);
        void ngx_event_add_timer(ngx_event_t *ev, ngx_msec_t timer);

        // process expired events
        void ngx_event_expire_timers(void);

        // assert only cancelable timers left
        ngx_int_t ngx_event_no_timers_left(void);

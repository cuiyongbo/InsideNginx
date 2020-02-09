**********************
Signal Processing Note
**********************

In nginx, processes are mainly of three kinds: master process, worker process and cache-related process.
And master process loads configuration, launches workers, and performs non-stop upgrade etc.
worker process is the main workhorse, processing income requests. and signals are used to communicate
with them. For example to perform upgrade, ``SIGUSR2`` are send to master process, and master process would
do some upgrade stuff when receiving this signal.

Following signals are handled with specific intentions, including:

.. code-block:: c

    #define ngx_signal_helper(n)     SIG##n
    #define ngx_signal_value(n)      ngx_signal_helper(n)

    #define NGX_SHUTDOWN_SIGNAL      QUIT
    #define NGX_TERMINATE_SIGNAL     TERM
    #define NGX_NOACCEPT_SIGNAL      WINCH
    #define NGX_RECONFIGURE_SIGNAL   HUP
    #define NGX_REOPEN_SIGNAL        USR1
    #define NGX_CHANGEBIN_SIGNAL     USR2

    ngx_int_t ngx_init_signals(ngx_log_t *log);
    static void ngx_signal_handler(int signo, siginfo_t *siginfo, void *ucontext);

and these dedicated designs have enabled nginx some fantastic powers, such
as worker supervision, non-stopping upgrade, exit with cleanup.

.. code-block:: sh
    :caption: Taken from *nginx-repo/auto/install*

    upgrade:
        $NGX_SBIN_PATH -t

        kill -USR2 \`cat $NGX_PID_PATH\`
        sleep 1
        test -f $NGX_PID_PATH.oldbin

        kill -QUIT \`cat $NGX_PID_PATH.oldbin\`

Source files include:

    - ngx_process.h, ngx_process.c
    - ngx_process_cycle.h, ngx_process_cycle.c
    - ngx_cycle.h, ngx_cycle.c

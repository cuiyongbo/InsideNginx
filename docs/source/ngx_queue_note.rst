****************
Nginx Queue Note
****************

In nginx, queue is implemented using **circular doubly linked list**.
Its implementation can be found in

    - ngx_queue.h, ngx_queue.c

Followings are structure defintion and pulic interfaces:

.. code-block:: c

    typedef struct ngx_queue_s  ngx_queue_t;

    struct ngx_queue_s {
        ngx_queue_t  *prev;
        ngx_queue_t  *next;
    };

    #define ngx_queue_init(q)                                                     \
        (q)->prev = q;                                                            \
        (q)->next = q

    #define ngx_queue_empty(h) (h == (h)->prev)

    #define ngx_queue_insert_head(h, x)                                           \
        (x)->next = (h)->next;                                                    \
        (x)->next->prev = x;                                                      \
        (x)->prev = h;                                                            \
        (h)->next = x

    #define ngx_queue_insert_after   ngx_queue_insert_head

    #define ngx_queue_insert_tail(h, x)                                           \
        (x)->prev = (h)->prev;                                                    \
        (x)->prev->next = x;                                                      \
        (x)->next = h;                                                            \
        (h)->prev = x

    #define ngx_queue_head(h) (h)->next
    #define ngx_queue_last(h) (h)->prev
    #define ngx_queue_sentinel(h) (h)
    #define ngx_queue_next(q) (q)->next
    #define ngx_queue_prev(q) (q)->prev

    #define ngx_queue_remove(x)                                                   \
        (x)->next->prev = (x)->prev;                                              \
        (x)->prev->next = (x)->next;                                              \
        (x)->prev = NULL;                                                         \
        (x)->next = NULL

    #define ngx_queue_data(q, type, link) (type *)((u_char *)q - offsetof(type, link))

    ngx_queue_t *ngx_queue_middle(ngx_queue_t *queue);

    typedef ngx_int_t (*queue_cmp_t)(const ngx_queue_t*, const ngx_queue_t*);
    void ngx_queue_sort(ngx_queue_t *queue, queue_cmp_t cmp);

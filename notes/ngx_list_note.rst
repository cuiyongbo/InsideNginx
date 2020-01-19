***************
Nginx List Note
***************

In nginx, list is implemented using singly linked list,
and it taked advantage of memory pool to reduce malloc-related
system calls. and source files are found:

   - ngx_list.h, ngx_list.c

Followings are structure definition and public interfaces:

.. code-block:: c

    typedef struct ngx_list_part_s  ngx_list_part_t;

    struct ngx_list_part_s {
        void*             elements;
        ngx_uint_t        elementCount;
        ngx_list_part_t  *next;
    };

    typedef struct {
        ngx_list_part_t*  last;
        ngx_list_part_t   part;
        size_t            elementSize;
        ngx_uint_t        capacity;
        ngx_pool_t       *pool;
    } ngx_list_t;

    ngx_list_t *ngx_list_create(ngx_pool_t *pool, ngx_uint_t n, size_t size);
    void *ngx_list_push(ngx_list_t *list);

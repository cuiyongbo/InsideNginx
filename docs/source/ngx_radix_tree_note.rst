*********************
Nginx Radix Tree Note
*********************

In nginx, radix tree is used in ``ngx_stream_geo_module``,
which is mainly used for *IP Routing and load balance* (???).
Its implementation can be found in

    - ngx_radix_tree.h, ngx_radix_tree.c

Followings are structure definition and public interfaces' declarations:

.. code-block:: c

    #define NGX_RADIX_NO_VALUE   (uintptr_t) -1

    typedef struct ngx_radix_node_s  ngx_radix_node_t;

    struct ngx_radix_node_s
    {
        ngx_radix_node_t  *right;
        ngx_radix_node_t  *left;
        ngx_radix_node_t  *parent;
        uintptr_t          value;
    };

    typedef struct {
        ngx_radix_node_t  *root;
        ngx_pool_t        *pool;

        // deleted node are reused in free list
        ngx_radix_node_t  *free_list;

        char              *start;
        size_t             size;
    } ngx_radix_tree_t;

    ngx_radix_tree_t *ngx_radix_tree_create(ngx_pool_t *pool, ngx_int_t preallocate);

    ngx_int_t ngx_radix32tree_insert(ngx_radix_tree_t *tree, uint32_t key, uint32_t mask, uintptr_t value);
    ngx_int_t ngx_radix32tree_delete(ngx_radix_tree_t *tree, uint32_t key, uint32_t mask);
    uintptr_t ngx_radix32tree_find(ngx_radix_tree_t *tree, uint32_t key);

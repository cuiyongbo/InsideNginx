****************
Nginx Array Note
****************

Array implementation is nothing new, I viewed it as a wrapper of ``int*``,
without considering memory allocation details. Its implementation can be found in

   - ngx_array.h, ngx_arrary.c

folllowings are structure definition and several interfaces used by callers:

.. code-block:: c

    typedef struct {
        void        *elements;
        ngx_uint_t   elementCount;
        size_t       elementSize;
        ngx_uint_t   capacity;
        ngx_pool_t  *pool;
    } ngx_array_t;

    ngx_array_t *ngx_array_create(ngx_pool_t *p, ngx_uint_t n, size_t size);
    void ngx_array_destroy(ngx_array_t *a);
    void *ngx_array_push(ngx_array_t *a);
    void *ngx_array_push_n(ngx_array_t *a, ngx_uint_t n);

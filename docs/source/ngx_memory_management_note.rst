**********************
Memory Management Note
**********************

In nginx, memory management is implemented using **memory pools**, which is implemented using
**singly linked list**. and has several benefits:

    - minimize memory leak
    - reduce the number of malloc-related system calls, thus reduce fragmentations
    - optimize memory allocation for small objects
    - improve memory access: memory addresses are aligned and there is a memory paging
      mechanism improving CPU page hits.

Source files:

    - ngx_palloc.h, ngx_palloc.c

.. code-block:: c

    ngx_pool_t* ngx_create_pool(size_t size, ngx_log_t *log)
    {
        // memory alignment
        ngx_pool_t* p = ngx_memalign(NGX_POOL_ALIGNMENT, size, log);

        // ...
    }

    void ngx_destroy_pool(ngx_pool_t *pool)
    {
        // ...

        // organized in singly linked list
        for (ngx_pool_large_t* l = pool->large; l; l = l->next)
        {
            if (l->alloc)
                ngx_free(l->alloc);
        }

        for (ngx_pool_t *p = pool, *n = pool->d.next; /* void */; p = n, n = n->d.next)
        {
            ngx_free(p);
            if (n == NULL)
                break;
        }
    }

    static void* ngx_palloc_block(ngx_pool_t* pool, size_t size)
    {
        // ...
        // similar to LRU (Least Recently Used) algorithm
        ngx_pool_t* p = pool->current;
        for (; p->d.next; p = p->d.next) {
            if (p->d.failed++ > 4)
                pool->current = p->d.next;
        }
        p->d.next = newPool;
        // ...
    }

    static void* ngx_palloc_large(ngx_pool_t *pool, size_t size)
    {
        // ...

        // access optimization
        ngx_uint_t n = 0;
        ngx_pool_large_t* large = pool->large;
        for (; large; large = large->next)
        {
            if (large->alloc == NULL)
            {
                large->alloc = newBlock;
                return newBlock;
            }

            if (n++ > 3)
                break;
        }

        large = ngx_palloc_small(pool, sizeof(ngx_pool_large_t), 1);
        large->alloc = newBlock;
        large->next = pool->large;
        pool->large = large;
        return newBlock;
    }


.. note::

    Different pool size may result in different memory usage. However, the less system malloc,
    the less memory fragmentations, which means the less memory usage.

.. rubric:: Footnotes

.. [#] `What is memory mapping <http://ecomputernotes.com/fundamental/input-output-and-memory/memory-mapping>`_

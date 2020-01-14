/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */

#include <ngx_config.h>
#include <ngx_core.h>

ngx_uint_t  ngx_pagesize;
ngx_uint_t  ngx_pagesize_shift;
ngx_uint_t  ngx_cacheline_size;

void* ngx_alloc(size_t size, ngx_log_t* log)
{
    void* p = malloc(size);
    if(p == NULL)
    {
        ngx_log_error(NGX_LOG_EMERG, log, ngx_errno, "malloc(%uz) failed", size);
    }
    ngx_log_debug2(NGX_LOG_DEBUG_ALLOC, log, 0, "malloc: %p:%uz", p, size);
}

void* ngx_calloc(size_t size, ngx_log_t* log)
{
    void* p = ngx_alloc(size, log);
    if (p) {
        ngx_memzero(p, size);
    }
    return p;
}

void* ngx_memalign(size_t alignment, size_t size, ngx_log_t* log)
{
    void  *p;
    int err = posix_memalign(&p, alignment, size);
    if (err) {
        ngx_log_error(NGX_LOG_EMERG, log, err,
                      "posix_memalign(%uz, %uz) failed", alignment, size);
        p = NULL;
    }
    ngx_log_debug3(NGX_LOG_DEBUG_ALLOC, log, 0,
                   "posix_memalign: %p:%uz @%uz", p, size, alignment);
    return p;
}
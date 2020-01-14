/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */

#ifndef _NGX_ALLOC_H_
#define _NGX_ALLOC_H_

#include <ngx_core.h>
#include <ngx_config.h>

void *ngx_alloc(size_t size, ngx_log_t *log);
void *ngx_calloc(size_t size, ngx_log_t *log);

#define ngx_free          free

void *ngx_memalign(size_t alignment, size_t size, ngx_log_t *log);

extern ngx_uint_t  ngx_pagesize;
extern ngx_uint_t  ngx_pagesize_shift;
extern ngx_uint_t  ngx_cacheline_size;

#endif

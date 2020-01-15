
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_ARRAY_H_INCLUDED_
#define _NGX_ARRAY_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


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


static ngx_inline ngx_int_t ngx_array_init(ngx_array_t *array, ngx_pool_t *pool, ngx_uint_t n, size_t size)
{
    array->elementCount = 0;
    array->elementSize = size;
    array->capacity = n;
    array->pool = pool;
	return (array->elements == NULL) ? NGX_ERROR : NGX_OK;
}


#endif /* _NGX_ARRAY_H_INCLUDED_ */

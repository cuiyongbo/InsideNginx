
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>


ngx_list_t* ngx_list_create(ngx_pool_t *pool, ngx_uint_t n, size_t size)
{
    ngx_list_t* list = ngx_palloc(pool, sizeof(ngx_list_t));
    if (list == NULL) {
        return NULL;
    }

    if (ngx_list_init(list, pool, n, size) != NGX_OK) {
        return NULL;
    }

    return list;
}

void* ngx_list_push(ngx_list_t *l)
{
    ngx_list_part_t* last = l->last;
    if (last->elementCount == l->capacity)
	{
        /* the last part is full, allocate a new list part */
        last = ngx_palloc(l->pool, sizeof(ngx_list_part_t));
        if (last == NULL) {
            return NULL;
        }

        last->elements = ngx_palloc(l->pool, l->capacity * l->elementSize);
        if (last->elements == NULL) {
            return NULL;
        }

        last->elementCount = 0;
        last->next = NULL;

		// push back
        l->last->next = last;
        l->last = last;
    }

    void* elt = (char *)last->elements + l->elementSize * last->elementCount;
    last->elementCount++;

    return elt;
}

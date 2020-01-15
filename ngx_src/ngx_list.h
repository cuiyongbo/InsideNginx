
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_LIST_H_INCLUDED_
#define _NGX_LIST_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


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

static ngx_inline ngx_int_t ngx_list_init(ngx_list_t *list, ngx_pool_t *pool, ngx_uint_t n, size_t size)
{
    list->part.elements = ngx_palloc(pool, n * size);
    if (list->part.elements == NULL) {
        return NGX_ERROR;
    }

    list->part.elementCount = 0;
    list->part.next = NULL;
    list->last = &list->part;
    list->elementSize = size;
    list->capacity = n;
    list->pool = pool;

    return NGX_OK;
}


/*
 *
 *  the iteration through the list:
 *
 *  part = &list.part;
 *  data = part->elements;
 *
 *  for (i = 0 ;; i++) {
 *
 *      if (i >= part->elementCount) {
 *          if (part->next == NULL) {
 *              break;
 *          }
 *
 *          part = part->next;
 *          data = part->elements;
 *          i = 0;
 *      }
 *
 *      ...  data[i] ...
 *
 *  }
 */


void *ngx_list_push(ngx_list_t *list);


#endif /* _NGX_LIST_H_INCLUDED_ */

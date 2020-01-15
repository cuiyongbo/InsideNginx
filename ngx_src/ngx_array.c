
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>


ngx_array_t* ngx_array_create(ngx_pool_t *p, ngx_uint_t n, size_t size)
{
    ngx_array_t* a = ngx_palloc(p, sizeof(ngx_array_t));
    if (a == NULL)
        return NULL;

    if (ngx_array_init(a, p, n, size) != NGX_OK)
        return NULL;

    return a;
}

void ngx_array_destroy(ngx_array_t *a)
{
    ngx_pool_t* p = a->pool;

	if ((u_char *) a->elements + a->elementSize * a->capacity == p->d.last)
        p->d.last -= a->elementSize * a->capacity;

    if ((u_char *) a + sizeof(ngx_array_t) == p->d.last)
        p->d.last = (u_char *) a;
}

void* ngx_array_push(ngx_array_t *a)
{
    if (a->elementCount == a->capacity)
	{
        /* the array is full */
		ngx_pool_t* p = a->pool;
		size_t size = a->elementSize * a->capacity;

        if ((u_char *)a->elements + size == p->d.last
            && p->d.last + a->elementSize <= p->d.end)
        {
            /*
             * the array allocation is the last in the pool
             * and there is space for new allocation
             */

            p->d.last += a->elementSize;
            a->capacity++;
        }
		else
		{
            /* allocate a new array */
            void* newArray = ngx_palloc(p, 2 * size);
            if (newArray == NULL)
                return NULL;

            ngx_memcpy(newArray, a->elements, size);
            a->elements = newArray;
            a->capacity *= 2;
        }
    }

    void* elt = (u_char*)a->elements + a->elementSize * a->elementCount;
    a->elementCount++;
    return elt;
}

void* ngx_array_push_n(ngx_array_t *a, ngx_uint_t n)
{
    size_t size = n * a->elementSize;
    if (a->elementCount + n > a->capacity)
	{
        /* the array is full */
		ngx_pool_t* p = a->pool;
        if ((u_char *)a->elements + a->elementSize * a->capacity == p->d.last
            && p->d.last + size <= p->d.end)
        {
            /*
             * the array allocation is the last in the pool
             * and there is space for new allocation
             */

            p->d.last += size;
            a->capacity += n;
        }
		else
		{
            /* allocate a new array */
            size_t nalloc = 2 * ((n >= a->capacity) ? n : a->capacity);
            void* newArray = ngx_palloc(p, nalloc * a->elementSize);
            if (newArray == NULL) {
                return NULL;
            }

            ngx_memcpy(newArray, a->elements, a->elementCount * a->elementSize);
            a->elements = newArray;
            a->capacity = nalloc;
        }
    }

    void* elt = (u_char *) a->elements + a->elementSize * a->elementCount;
    a->elementCount += n;
    return elt;
}

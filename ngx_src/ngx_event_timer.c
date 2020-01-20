
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_event.h>


ngx_rbtree_t              ngx_event_timer_rbtree;
static ngx_rbtree_node_t  ngx_event_timer_sentinel;

/*
 * the event timer rbtree may contain the duplicate keys, however,
 * it should not be a problem, because we use the rbtree to find
 * a minimum timer value only
 */

ngx_int_t ngx_event_timer_init(ngx_log_t *log)
{
	ngx_rbtree_init(&ngx_event_timer_rbtree, 
					&ngx_event_timer_sentinel,
					ngx_rbtree_insert_timer_value);

	return NGX_OK;
}

ngx_msec_t ngx_event_find_timer(void)
{
	if (ngx_event_timer_rbtree.root == &ngx_event_timer_sentinel)
		return NGX_TIMER_INFINITE;

	ngx_rbtree_node_t *root = ngx_event_timer_rbtree.root;
	ngx_rbtree_node_t *sentinel = ngx_event_timer_rbtree.sentinel;
	ngx_rbtree_node_t *node = ngx_rbtree_min(root, sentinel);
	ngx_msec_int_t timer = (ngx_msec_int_t)(node->key - ngx_current_msec);
	return (ngx_msec_t)(timer > 0 ? timer : 0);
}

void ngx_event_expire_timers(void)
{
	ngx_rbtree_node_t *sentinel = ngx_event_timer_rbtree.sentinel;
	for ( ;; )
	{
		ngx_rbtree_node_t *root = ngx_event_timer_rbtree.root;
		if (root == sentinel)
			return;

		ngx_rbtree_node_t *node = ngx_rbtree_min(root, sentinel);
		if ((ngx_msec_int_t) (node->key - ngx_current_msec) > 0)
			return;

		ngx_event_t *ev = (ngx_event_t *)((char *)node - offsetof(ngx_event_t, timer));

		ngx_log_debug2(NGX_LOG_DEBUG_EVENT, ev->log, 0,
					   "event timer del: %d: %M",
					   ngx_event_ident(ev->data), ev->timer.key);

		ngx_rbtree_delete(&ngx_event_timer_rbtree, &ev->timer);

		ev->timer_set = 0;
		ev->timedout = 1;
		ev->handler(ev);
	}
}

ngx_int_t ngx_event_no_timers_left(void)
{
	ngx_rbtree_node_t *root = ngx_event_timer_rbtree.root;
	ngx_rbtree_node_t *sentinel = ngx_event_timer_rbtree.sentinel;
	if (root == sentinel)
		return NGX_OK;

	for (ngx_rbtree_node_t *node = ngx_rbtree_min(root, sentinel);
		 node;
		 node = ngx_rbtree_next(&ngx_event_timer_rbtree, node))
	{
		ngx_event_t* ev = (ngx_event_t *)((char *)node - offsetof(ngx_event_t, timer));
		if (!ev->cancelable)
			return NGX_AGAIN;
	}

	/* only cancelable timers left */

	return NGX_OK;
}

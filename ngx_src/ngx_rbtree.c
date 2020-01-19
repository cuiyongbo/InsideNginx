
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>


/*
 * The red-black tree code is based on the algorithm described in
 * the "Introduction to Algorithms" by Cormen, Leiserson and Rivest.
 */


static ngx_inline void ngx_rbtree_left_rotate(ngx_rbtree_node_t **root, ngx_rbtree_node_t *sentinel, ngx_rbtree_node_t *node);
static ngx_inline void ngx_rbtree_right_rotate(ngx_rbtree_node_t **root, ngx_rbtree_node_t *sentinel, ngx_rbtree_node_t *node);


void ngx_rbtree_insert(ngx_rbtree_t *tree, ngx_rbtree_node_t *node)
{
	ngx_rbtree_node_t  **root = &tree->root;
	ngx_rbtree_node_t *sentinel = tree->sentinel;

	if (*root == sentinel)
	{
		node->parent = NULL;
		node->left = sentinel;
		node->right = sentinel;
		ngx_rbt_black(node);
		*root = node;
		return;
	}

	/* a binary tree insert */
	tree->insert(*root, node, sentinel);

	/* re-balance tree */
	while (node != *root && ngx_rbt_is_red(node->parent))
	{
		// grandparent must be black
		ngx_rbtree_node_t *uncle = NULL;
		if (node->parent == node->parent->parent->left)
		{
			uncle = node->parent->parent->right;
			if (ngx_rbt_is_red(uncle))
			{
				ngx_rbt_black(node->parent);
				ngx_rbt_black(uncle);
				ngx_rbt_red(node->parent->parent);
				node = node->parent->parent;
			}
			else
			{
				if (node == node->parent->right)
				{
					node = node->parent;
					ngx_rbtree_left_rotate(root, sentinel, node);
				}

				ngx_rbt_black(node->parent);
				ngx_rbt_red(node->parent->parent);
				ngx_rbtree_right_rotate(root, sentinel, node->parent->parent);
			}
		}
		else
		{
			uncle = node->parent->parent->left;

			if (ngx_rbt_is_red(uncle))
			{
				ngx_rbt_black(node->parent);
				ngx_rbt_black(uncle);
				ngx_rbt_red(node->parent->parent);
				node = node->parent->parent;

			}
			else
			{
				if (node == node->parent->left)
				{
					node = node->parent;
					ngx_rbtree_right_rotate(root, sentinel, node);
				}

				ngx_rbt_black(node->parent);
				ngx_rbt_red(node->parent->parent);
				ngx_rbtree_left_rotate(root, sentinel, node->parent->parent);
			}
		}
	}

	ngx_rbt_black(*root);
}

void ngx_rbtree_insert_value(ngx_rbtree_node_t *temp, ngx_rbtree_node_t *node, ngx_rbtree_node_t *sentinel)
{
	ngx_rbtree_node_t  **p;

	for ( ;; ) {

		p = (node->key < temp->key) ? &temp->left : &temp->right;

		if (*p == sentinel)
			break;

		temp = *p;
	}

	*p = node;
	node->parent = temp;
	node->left = sentinel;
	node->right = sentinel;
	ngx_rbt_red(node);
}

void ngx_rbtree_insert_timer_value(ngx_rbtree_node_t *temp, ngx_rbtree_node_t *node, ngx_rbtree_node_t *sentinel)
{
	ngx_rbtree_node_t  **p;

	for ( ;; )
	{

		/*
		 * Timer values
		 * 1) are spread in small range, usually several minutes,
		 * 2) and overflow each 49 days, if milliseconds are stored in 32 bits.
		 * The comparison takes into account that overflow.
		 */

		/*  node->key < temp->key */

		p = ((ngx_rbtree_key_int_t)(node->key - temp->key) < 0)
				? &temp->left : &temp->right;

		if (*p == sentinel)
			break;

		temp = *p;
	}

	*p = node;
	node->parent = temp;
	node->left = sentinel;
	node->right = sentinel;
	ngx_rbt_red(node);
}

static void ngx_rbtree_transplant(ngx_rbtree_t *tree, ngx_rbtree_t* u, ngx_rbtree_t* v)
{
	if (u->p == tree->sentinel)
	{
		tree->root = v;
	}
	else if (u == u->parent->left)
	{
		u->parent->left = v;
	}
	else
	{
		u->parent->right = v;
	}

	v->parent = u->parent;
}

void ngx_rbtree_delete(ngx_rbtree_t *tree, ngx_rbtree_node_t *node)
{
	ngx_rbtree_node_t  **root = &tree->root;
	ngx_rbtree_node_t  *sentinel = tree->sentinel;

	ngx_rbtree_node_t* x = NULL;
	ngx_rbtree_node_t* y = node;
	ngx_uint_t originColorIsRed = ngx_rbt_is_red(y);

	if (node->left == sentinel)
	{
		x = node->right;
		ngx_rbtree_transplant(tree, node, node->right);
	}
	else if (node->right == sentinel)
	{
		x = node->left;
		ngx_rbtree_transplant(tree, node, node->left);
	}
	else
	{
		y = ngx_rbtree_min(node->right, sentinel);
		originColorIsRed = ngx_rbt_is_red(y->color);
		x = y->right;
		if (y == node->right)
		{
			x->parent = y;
		}
		else
		{
			ngx_rbtree_transplant(tree, y, y->right);
			y->right = node->right;
			node->right->parent = y;
		}

		ngx_rbtree_transplant(tree, node, y);
		y->left = node->left;
		node->left->parent = y;
		ngx_rbt_copy_color(y, node);
	}

	/* DEBUG stuff */
	node->left = NULL;
	node->right = NULL;
	node->parent = NULL;
	node->key = 0;

	if (originColorIsRed)
		return;
	
	/* a delete fixup */
	while (x != *root && ngx_rbt_is_black(x))
	{
		// sibling can not be root->sentinel
		ngx_rbtree_node_t* sibling;
		if (x == x->parent->left)
		{
			sibling = x->parent->right;

			if (ngx_rbt_is_red(sibling))
			{
				// case 1
				ngx_rbt_black(sibling);
				ngx_rbt_red(x->parent);
				ngx_rbtree_left_rotate(root, sentinel, x->parent);
				sibling = x->parent->right;
			}

			if (ngx_rbt_is_black(sibling->left) && ngx_rbt_is_black(sibling->right))
			{
				// case 2
				ngx_rbt_red(sibling);
				x = x->parent;
			}
			else
			{
				if (ngx_rbt_is_black(sibling->right))
				{
					// case 3
					ngx_rbt_black(sibling->left);
					ngx_rbt_red(sibling);
					ngx_rbtree_right_rotate(root, sentinel, sibling);
					sibling = x->parent->right;
				}

				// case 4
				ngx_rbt_copy_color(sibling, x->parent);
				ngx_rbt_black(x->parent);
				ngx_rbt_black(sibling->right);
				ngx_rbtree_left_rotate(root, sentinel, x->parent);
				x = *root;
			}
		}
		else
		{
			sibling = x->parent->left;

			if (ngx_rbt_is_red(sibling))
			{
				ngx_rbt_black(sibling);
				ngx_rbt_red(x->parent);
				ngx_rbtree_right_rotate(root, sentinel, x->parent);
				sibling = x->parent->left;
			}

			if (ngx_rbt_is_black(sibling->left) && ngx_rbt_is_black(sibling->right))
			{
				ngx_rbt_red(sibling);
				x = x->parent;

			}
			else
			{
				if (ngx_rbt_is_black(sibling->left))
				{
					ngx_rbt_black(sibling->right);
					ngx_rbt_red(sibling);
					ngx_rbtree_left_rotate(root, sentinel, sibling);
					sibling = x->parent->left;
				}

				ngx_rbt_copy_color(sibling, x->parent);
				ngx_rbt_black(x->parent);
				ngx_rbt_black(sibling->left);
				ngx_rbtree_right_rotate(root, sentinel, x->parent);
				x = *root;
			}
		}
	}

	ngx_rbt_black(x);
}

static ngx_inline void ngx_rbtree_left_rotate(ngx_rbtree_node_t **root, ngx_rbtree_node_t *sentinel, ngx_rbtree_node_t *node)
{
	ngx_rbtree_node_t* temp = node->right;
	node->right = temp->left;

	if (temp->left != sentinel)
	{
		temp->left->parent = node;
	}

	temp->parent = node->parent;

	if (node == *root)
		*root = temp;
	else if (node == node->parent->left)
		node->parent->left = temp;
	else
		node->parent->right = temp;

	temp->left = node;
	node->parent = temp;
}

static ngx_inline void ngx_rbtree_right_rotate(ngx_rbtree_node_t **root, ngx_rbtree_node_t *sentinel, ngx_rbtree_node_t *node)
{
	ngx_rbtree_node_t* x = node->left;
	node->left = x->right;

	if (x->right != sentinel)
	{
		x->right->p = node;
	}

	if (node == *root)
	{
		*root = x;
	}
	else if (node == node->parent->left)
	{
		node->parent->left = x;
	}
	else
	{
		node->parent->right = x;
	}
	
	x->right = node;
	node->parent = x;
}

ngx_rbtree_node_t * ngx_rbtree_next(ngx_rbtree_t *tree, ngx_rbtree_node_t *node)
{
	ngx_rbtree_node_t *sentinel = tree->sentinel;
	if (node->right != sentinel)
		return ngx_rbtree_min(node->right, sentinel);

	ngx_rbtree_node_t* parent = node->parent;
	while (parent != NULL && node != parent->left)
	{
		node = parent;
		parent = node->parent;
	}
	return parent;
}

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "bintree.h"
#include "anchor.h"

BinTree*
bintree_new_node(Anchor *anchor)
{
	BinTree *ret;

	ret = (BinTree *)malloc(sizeof(BinTree));
	if (ret == NULL) {
		debug("malloc(): %s\n", strerror(errno));
		return NULL;
	}

	memset(ret, 0, sizeof(BinTree));
	if (anchor)
		memcpy(&ret->anchor, anchor, sizeof(Anchor));

	return ret;
}

BinTree*
bintree_add_node(BinTree *root, BinTree *node)
{
	int t;

	t = strcmp(root->anchor.name, node->anchor.name);
	if (t == 0)
		return root;

	if (t > 0) {
		if (root->right == NULL) {
			root->right = node;
			return node;
		}

		return bintree_add_node(root->right, node);
	}

	if (root->left == NULL) {
		root->left = node;
		return node;
	}

	return bintree_add_node(root->left, node);
}

void
bintree_delete_node(BinTree *node)
{
	if (node == NULL)
		return;

	free(node);
}

void
bintree_delete_tree(BinTree *root)
{
	if (root == NULL)
		return;

	bintree_delete_tree(root->left);
	bintree_delete_tree(root->right);
	bintree_delete_node(root);
}

BinTree*
bintree_search(BinTree *root, char *name)
{
	int t;

	if (root == NULL)
		return NULL;

	t = strcmp(root->anchor.name, name);
	if (t == 0)
		return root;

	if (t > 0)
		return bintree_search(root->right, name);

	return bintree_search(root->left, name);
}

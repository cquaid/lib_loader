#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "bintree.h"

BinTree*
bintree_new_node(char *name, unsigned long hash)
{
	BinTree *ret;
	size_t len;

	ret = (BinTree *)malloc(sizeof(BinTree));
	if (ret == NULL) {
		debug("%s: malloc(ret): %s\n", __func__,
			  strerror(errno));
		return NULL;
	}

	memset(ret, 0, sizeof(BinTree));
	
	len = strlen(name);
	ret->name = (char *)malloc(len + 1);
	if (ret->name == NULL) {
		free(ret);
		debug("%s: malloc(name): %s\n", __func__,
			  strerror(errno));
		return NULL;
	}

	strncpy(ret->name, name, len);
	ret->name[len] = '\0';

	ret->hash = hash;

	return ret;
}

BinTree*
bintree_add_node(BinTree *root, BinTree *node)
{
	int t;

	t = strcmp(root->name, node->name);
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
	
	if (node->name != NULL)
		free(node->name);
	
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

	t = strcmp(root->name, name);
	if (t == 0)
		return root;
	
	if (t > 0)
		return bintree_search(root->right, name);
	
	return bintree_search(root->left, name); 
}

#ifndef _H_BINTREE
#define _H_BINTREE

#include "../rtld/anchor.h"

struct _bintree {
	Anchor anchor;
	struct _bintree *left;
	struct _bintree *right;
};
typedef struct _bintree BinTree;

extern BinTree* bintree_new_node(Anchor *anchor);
extern BinTree* bintree_add_node(BinTree *root, BinTree *node);
extern void bintree_delete_node(BinTree *node);
extern void bintree_delete_tree(BinTree *root);
extern BinTree* bintree_search(BinTree *root, char *name);

#endif /* _H_BINTREE */

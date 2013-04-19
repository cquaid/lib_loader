#ifndef _H_BINTREE
#define _H_BINTREE

struct _bintree {
	char *name;
	unsigned long hash; /* unused */
	struct _bintree *left;
	struct _bintree *right;
};
typedef struct _bintree BinTree;

extern BinTree* bintree_new_node(char *name, unsigned long hash);
extern BinTree* bintree_add_node(BinTree *root, BinTree *node);
extern void bintree_delete_node(BinTree *node);
extern void bintree_delete_tree(BinTree *root);
extern BinTree* bintree_search(BinTree *root, char *name);

#endif /* _H_BINTREE */

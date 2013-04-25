#include <stddef.h>
#include <string.h>

#include "bintree.h"
#include "debug.h"
#include "list.h"
#include "rtld.h"

static BinTree *fixup_tree = NULL;
static List *object_list = NULL;


/**
 * Dummy Functions
 */
static void free_object_list(void *data){}


void
add_fixup_anchor(Anchor *anchor)
{
	BinTree *tmp;

	if (anchor == NULL)
		return;

	/* create a new tree if one doesn't exist */
	if (fixup_tree == NULL) {
		fixup_tree = bintree_new_node(anchor);
		if (fixup_tree == NULL) {
			debugln("couldn't allocate the fixup_tree");
			return;
		}
		return;
	}

	/* check if the anchor is already in the tree */
	tmp = bintree_search(fixup_tree, anchor->name);
	if (tmp != NULL) {
		debug("%s: replacing `%s' with new definition\n",
			  __func__, anchor->name);
		/* replace the old anchor with the new one */
		memcpy(&tmp->anchor, anchor, sizeof(Anchor));
		return;
	}

	/* if not found, add a new node to the tree */
	tmp = bintree_new_node(anchor);
	if (tmp == NULL) {
		debugln("couldn't allocate a bintree node");
		return;
	}

	(void)bintree_add_node(fixup_tree, tmp);
}

void*
fixup_lookup(char *name)
{
	BinTree *b;
	ListNode *c;
	
	if (fixup_tree == NULL && object_list == NULL) {
#if 0
		debug("%s: WARN: `%s' not found\n", __func__, name);
#endif
		return NULL;
	}
	
	if (fixup_tree == NULL)
		goto object_search;

	b = bintree_search(fixup_tree, name);
	if (b != NULL) {
		debug("%s: INFO: `%s' found in fixup_tree\n", __func__, name);
		return b->anchor.symbol;
	}

object_search:
	if (object_list == NULL)
		goto out;

	c = object_list->head;
	for (; c != NULL; c = c->next) {
		void *d = elf_dlsym((elf_object *)(c->data), name);
		if (d != NULL) {
			debug("%s: INFO: `%s' found in object_list\n", __func__, name);
			return d;
		}
	}

out:
#if 0
	debug("%s: WARN: `%s' not found\n", __func__, name);
#endif
	return NULL;
}

void
add_object_list(elf_object *obj)
{
	ListNode *tmp;

	if (obj == NULL)
		return;

	if (object_list == NULL) {
		object_list = ll_new_list();
		if (object_list == NULL) {
			debugln("couldn't allocate new object_list");
			return;
		}
	}

	tmp = ll_new_node((void*)obj);
	if (tmp == NULL) {
		debugln("couldn't allocate new list node");
		return;
	}
	
	ll_push_node(object_list, tmp);
}

void
cleanup_object_list(void)
{
	ll_delete_list(object_list, free_object_list);
	object_list = NULL;
}

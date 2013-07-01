#ifndef _H_ANCHOR
#define _H_ANCHOR

struct _anchor {
	char *name;
	void *symbol;
};
typedef struct _anchor Anchor;

#endif /* _H_ANCHOR */

/*
 *	When the loader encounters an external symbol it calls the
 *	fixup function to resolve it.  The fixup function has to
 *	be aware of already loaded symbols and where to resolve them
 *	to.  That's where the Anchors come in.  The fixup function
 *	searches through a binary search tree for an anchor that matches
 *  the function name.  Once found, the symbol member is returned.
 *
 *	The Anchor tree superceed any symbols from already loaded
 *	shared objects.  So if you want to hook the malloc function,
 *	for example, you can add a replacement function to the list
 *	and when it goes to resolve malloc, it'll call the given
 *	definition instead of one found in a loaded object.
 */

#ifndef _H_LIST
#define _H_LIST

#include <stddef.h>

struct _llist_node {
	void *data;
	struct _llist_node *next;
};
typedef struct _llist_node ListNode;

struct _llist {
	ListNode *head;
	ListNode *tail;
	ListNode *current;
	size_t size;
};
typedef struct _llist List;

typedef void(*ll_free)(void*);
typedef int(*ll_compare)(void*, void*);

extern void ll_push_node(List *ll, ListNode *n);
extern ListNode* ll_pop_node(List *ll);

extern List *ll_new_list(void);
extern ListNode* ll_new_node(void *data);

extern void ll_delete_list(List *ll, ll_free nfree);
extern void ll_delete_node(ListNode *n, ll_free nfree);
extern void ll_delete_node_chain(ListNode *n, ll_free nfree);

extern ListNode* ll_get_next(List *ll);
extern ListNode* ll_find_node(List *ll, void *data, ll_compare ncmp);

#define ll_head_list(ll)  (ll->head)
#define ll_tail_list(ll)  (ll->tail)
#define ll_size_list(ll)  (ll->size)
#define ll_reset_list(ll) do{ll->current = ll->head;}while(0)

#endif /* _H_LIST */

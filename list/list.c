#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"

List*
ll_new_list(void)
{
	List *ret = (List*)malloc(sizeof(List));
	if (ret == NULL)
		return NULL;
	
	memset(ret, 0, sizeof(List));

	return ret;
}

ListNode*
ll_new_node(void *data)
{
	ListNode *ret = (ListNode*)malloc(sizeof(ListNode));
	if (ret == NULL)
		return NULL;
	
	memset(ret, 0, sizeof(ListNode));
	ret->data = data;

	return ret;
}

void
ll_push_node(List *ll, ListNode *n)
{
	if (ll == NULL || n == NULL)
		return;
	
	if (ll->size == 0) {
		ll->head = n;
		ll->tail = n;
		goto walk;
	}

	ll->tail->next = n;
	ll->tail = ll->tail->next;

walk:
	ll->size++;
	while (ll->tail->next != NULL) {
		ll->tail = ll->tail->next;
		ll->size++;
	}
}

ListNode*
ll_pop_node(List *ll)
{
	ListNode *tmp;
	ListNode *ret;

	if (ll == NULL)
		return NULL;

	if (ll->size == 0)
		return NULL;

	ret = ll->tail;

	if (ret == ll->head) {
		ll->tail = NULL;
		ll->head = NULL;
		ll->size = 0;
		return ret;
	}

	tmp = ll->head;
	while (tmp->next != ret)
		tmp = tmp->next;

	tmp->next = NULL;
	ll->tail = tmp;
	ll->size--;

	return ret;
}

void
ll_delete_list(List *ll, ll_free nfree)
{
	if (ll == NULL)
		return;

	if (ll->size == 0) {
		free(ll);
		return;
	}

	ll_delete_node_chain(ll->head, nfree);

	free(ll);
}

void
ll_delete_node(ListNode *n, ll_free nfree)
{
	if (n == NULL)
		return;

	if (n->data) {
		if (nfree)
			nfree(n->data);
		else
			free(n->data);
	}
	
	free(n);
}

void
ll_delete_node_chain(ListNode *root, ll_free nfree)
{
	ListNode *cur;
	ListNode *nxt;

	if (root == NULL)
		return;

	cur = root;

	while (cur) {
		nxt = cur->next;
		ll_delete_node(cur, nfree);
		cur = nxt;
	}
}

ListNode*
ll_get_next(List *ll)
{
	ListNode *ret;

	if (ll == NULL)
		return NULL;
	
	if (ll->current == ll->tail)
		return NULL;
	
	if (ll->current == NULL) {
		ll->current = ll->head;
		return ll->current;
	}

	ret = ll->current;
	ll->current = ll->current->next;

	return ret;
}

ListNode*
ll_find_node(List *ll, void *data, ll_compare ncmp)
{
	ListNode *res;
	ListNode *tmp;

	if (ll == NULL)
		return NULL;
	
	res = ll->current;
	ll->current = ll->head;

	while ((tmp = ll_get_next(ll)) != NULL) {
		if (ncmp) {
			if (ncmp(tmp->data, data)) {
				ll->current = res;
				return tmp;
			}
		}
		else {
			if (tmp->data == data) {
				ll->current = res;
				return tmp;
			}
		}
	}

	ll->current = res;
	return NULL;
}

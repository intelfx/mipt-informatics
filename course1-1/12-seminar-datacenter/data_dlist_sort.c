#include <assert.h>
#include <stdlib.h>

void
list_link_two(struct Node *_1, struct Node *_2)
{
    if (_1 != NULL) {
	_1->next = _2;
    }
    if (_2 != NULL) {
	_2->prev = _1;
    }
}

void
list_link_three(struct Node *_1, struct Node *_2, struct Node *_3)
{
    list_link_two(_1, _2);
    list_link_two(_2, _3);
}

void
list_sort(struct Node **list)
{
    if (*list == NULL || (*list)->next == NULL) {
	return;
    }

    /*
     * a stub head for the source list 
     */
    struct Node     src_list;
    list_link_three(NULL, &src_list, *list);

    struct Node    *sorted_list = NULL;

    while (src_list.next != NULL) {
	/*
	 * find maximal element in the remaining unsorted list 
	 */
	struct Node    *max = src_list.next;
	for (struct Node * cur = max->next; cur != NULL; cur = cur->next) {
	    if (cur->val > max->val) {
		max = cur;
	    }
	}

	/*
	 * move the found element into the beginning of the sorted list 
	 */
	struct Node    *tmp = max;

	/*
	 * remove from the old list... 
	 */
	list_link_two(tmp->prev, tmp->next);
	if ((sorted_list == NULL) || (tmp->val != sorted_list->val)) {
	    /*
	     * ...insert into the new list 
	     */
	    list_link_three(NULL, tmp, sorted_list);
	    sorted_list = tmp;
	} else {
	    /*
	     * ...and remove the duplicate 
	     */
	    free(tmp);
	}
    }

    *list = sorted_list;
}

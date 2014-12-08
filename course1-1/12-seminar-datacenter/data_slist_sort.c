#include <assert.h>
#include <stdlib.h>

void list_sort(struct Node **list)
{
    if (*list == NULL || (*list)->next == NULL) {
	return;
    }

    struct Node *sorted_list = NULL;

    while (*list != NULL) {
	/* find maximal element in the remaining unsorted list */
	struct Node **max = list;
	for (struct Node ** cur = &(*list)->next; *cur != NULL;
	     cur = &(*cur)->next) {
	    if ((*cur)->val > (*max)->val) {
		max = cur;
	    }
	}

	/* move the found element into the beginning of the sorted list */
	struct Node *tmp = *max;

	/* remove from the old list... */
	*max = tmp->next;
	if ((sorted_list == NULL) || (tmp->val != sorted_list->val)) {
	    /* ...insert into the new list */
	    tmp->next = sorted_list;
	    sorted_list = tmp;
	} else {
	    /* ...and remove the duplicate */
	    free(tmp);
	}
    }

    *list = sorted_list;
}

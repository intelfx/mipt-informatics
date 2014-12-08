#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

struct List* list_create()
{
    return (struct List*) calloc (1, sizeof (struct List));
}

void list_add_first (struct List* list, Data x)
{
    struct Node* node = (struct Node*) malloc (sizeof (struct Node));
    node->val = x;

    node->next = list->head;
    list->head = node;
}

void list_add_last (struct List* list, Data x)
{
    struct Node* node = (struct Node*) malloc (sizeof (struct Node));
    node->val = x;
    node->next = 0;

    struct Node* last = list->head;
    if (last) {
        while (last->next) {
            last = last->next;
        }
        last->next = node;
    } else {
        list->head = node;
    }
}

Data list_remove_first (struct List* list)
{
    struct Node* next = list->head->next;
    Data val = list->head->val;
    free (list->head);
    list->head = next;
    return val;
}

Data list_remove_last (struct List* list)
{
    struct Node* last = list->head->next;
    Data val;
    if (last) {
        struct Node* prev = list->head;
        while (last->next) {
            prev = last;
            last = last->next;
        }
        val = last->val;
        free (last);
        prev->next = 0;
    } else {
        val = list->head->val;
        free (list->head);
        list->head = 0;
    }
    return val;
}

Data list_get_first (struct List* list)
{
    return list->head->val;
}

Data list_get_last (struct List* list)
{
    struct Node* last = list->head;
    while (last->next) {
        last = last->next;
    }
    return last->val;
}

void list_print (struct List* list)
{
    struct Node* cur = list->head;
    if (cur) {
        do {
            printf ("%d ", cur->val);
            cur = cur->next;
        } while (cur);
        putchar ('\n');
    } else {
        printf ("Empty list\n");
    }
}

int list_size (struct List* list)
{
    int size = 0;
    struct Node* cur = list->head;
    while (cur) {
        ++size;
        cur = cur->next;
    }
    return size;
}

void list_clear (struct List* list)
{
    struct Node* cur = list->head;
    list->head = 0;
    while (cur) {
        struct Node* next = cur->next;
        free (cur);
        cur = next;
    }
}

void list_destroy (struct List* list)
{
    list_clear (list);
    free (list);
}

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

typedef int Data;
struct Node {
    struct Node* next;
    struct Node* prev;
    Data data;
};

void list_init (struct Node* list)
{
    list->prev = list->next = list;
}

void list_insert (struct Node* list, struct Node* t)
{
    t->prev = list;
    t->next = list->next;
    t->prev->next = t;
    t->next->prev = t;
}

void list_insert_before (struct Node* list, struct Node* t)
{
    t->prev = list->prev;
    t->next = list;
    t->prev->next = t;
    t->next->prev = t;
}

void list_remove (struct Node* t)
{
    struct Node *prev = t->prev, *next = t->next;
    prev->next = next;
    next->prev = prev;
}

struct Node* list_push_front (struct Node* list, Data d)
{
    struct Node* node = malloc (sizeof (struct Node));
    node->data = d;
    list_insert (list, node);
    return node;
}

struct Node* list_push_back (struct Node* list, Data d)
{
    struct Node* node = malloc (sizeof (struct Node));
    node->data = d;
    list_insert_before (list, node);
    return node;
}

Data list_delete (struct Node* t)
{
    Data val = t->data;
    list_remove (t);
    free (t);
    return val;
}

Data list_pop_front (struct Node* list)
{
    return list_delete (list->next);
}

Data list_pop_back (struct Node* list)
{
    return list_delete (list->prev);
}

int list_is_empty (struct Node* list)
{
    return (list->prev == list) && (list->next == list);
}

int list_clear (struct Node* list)
{
    struct Node* cur = list->next;
    while (cur != list)
    {
        struct Node* next = cur->next;
        free (cur);
        cur = next;
    }
    list_init (list);
    return 0;
}

struct Node* advance (struct Node* list, struct Node* ptr)
{
    struct Node* next = ptr->next;
    if (next == list) {
        next = next->next;
    }
    return next;
}

int main()
{
    int people, words;
    scanf ("%d %d", &people, &words);

    struct Node list;
    list_init (&list);
    for (int i = 0; i < people; ++i) {
        list_push_back (&list, i+1);
    }

    int last_removed[3];
    int list_size = people;
    struct Node* ptr = list.next;
    while (!list_is_empty (&list)) {
        for (int i = 1; i < words; ++i) {
            ptr = advance (&list, ptr);
        }

        struct Node *next = advance (&list, ptr);
        int value = list_delete (ptr);
        ptr = next;
        --list_size;

        if (list_size < 3) {
            last_removed[list_size] = value;
        }
    }

    for (int i = 2; i >= 0; --i) {
        printf ("%d ", last_removed[i]);
    }
    putchar ('\n');
}

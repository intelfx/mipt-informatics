#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

typedef int Data;

struct Node {
    struct Node *next, *prev;
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

void list_foreach (struct Node* list, void (*func) (Data d, void* param), void* param)
{
    struct Node* cur = list->next;
    while (cur != list) {
        func (cur->data, param);
        cur = cur->next;
    }
}

static void list_print_cb (Data d, void* param)
{
    printf ("%d ", d);
}

void list_print (struct Node* list)
{
    list_foreach (list, &list_print_cb, 0);
    putchar ('\n');
}

static void list_sum_cb (Data d, void* param)
{
    *((Data*)param) += d;
}

Data list_sum (struct Node* list)
{
    Data sum = 0;
    list_foreach (list, &list_sum_cb, (void*)&sum);
    return sum;
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

static const int INITIAL_ELEMENT_COUNT = 5;

int compare_values (int c[2]) {
    assert (c[0] != c[1]);
    if ((c[0] == 0) && (c[1] == 9)) {
        return 0;
    }
    if ((c[1] == 0) && (c[0] == 9)) {
        return 1;
    }
    if (c[0] > c[1]) {
        return 0;
    } else {
        return 1;
    }
}

int main()
{
    struct Node player[2];

    for (int i = 0; i < 2; ++i) {
        list_init (&player[i]);

        for (int j = 0; j < INITIAL_ELEMENT_COUNT; ++j) {
            int elem;
            scanf (" %d", &elem);
            list_push_back (&player[i], elem);
        }
    }

    int turn_count = 0;
    while (!list_is_empty (&player[0]) &&
           !list_is_empty (&player[1]) &&
           (turn_count < 1e6)) {
        int top_cards[2];
        int winning;
        ++turn_count;
        for (int i = 0; i < 2; ++i) {
            top_cards[i] = list_pop_front (&player[i]);
        }
        winning = compare_values (top_cards);
        for (int i = 0; i < 2; ++i) {
            list_push_back (&player[winning], top_cards[i]);
        }
    }

    if (list_is_empty (&player[0]) ||
        list_is_empty (&player[1])) {
        printf ("%s %d\n",
                list_is_empty (&player[0]) ? "second" : "first",
                turn_count);
    } else {
        printf ("botva\n");
    }

    list_clear (&player[0]);
    list_clear (&player[1]);
}

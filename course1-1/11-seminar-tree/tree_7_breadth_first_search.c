#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

/*
 * Implementation of the node queue: double linked list.
 */

typedef struct TreeNode* ListData;

struct ListNode {
    struct ListNode *next, *prev;
    ListData data;
};

void list_init (struct ListNode* list)
{
    list->prev = list->next = list;
}

void list_insert (struct ListNode* list, struct ListNode* t)
{
    t->prev = list;
    t->next = list->next;
    t->prev->next = t;
    t->next->prev = t;
}

void list_insert_before (struct ListNode* list, struct ListNode* t)
{
    t->prev = list->prev;
    t->next = list;
    t->prev->next = t;
    t->next->prev = t;
}

void list_remove (struct ListNode* t)
{
    struct ListNode *prev = t->prev, *next = t->next;
    prev->next = next;
    next->prev = prev;
}

struct ListNode* list_push_front (struct ListNode* list, ListData d)
{
    struct ListNode* node = malloc (sizeof (struct ListNode));
    node->data = d;
    list_insert (list, node);
    return node;
}

struct ListNode* list_push_back (struct ListNode* list, ListData d)
{
    struct ListNode* node = malloc (sizeof (struct ListNode));
    node->data = d;
    list_insert_before (list, node);
    return node;
}

ListData list_delete (struct ListNode* t)
{
    ListData val = t->data;
    list_remove (t);
    free (t);
    return val;
}

ListData list_pop_front (struct ListNode* list)
{
    return list_delete (list->next);
}

ListData list_pop_back (struct ListNode* list)
{
    return list_delete (list->prev);
}

void list_foreach (struct ListNode* list, void (*func) (ListData d, void* param), void* param)
{
    struct ListNode* cur = list->next;
    while (cur != list) {
        func (cur->data, param);
        cur = cur->next;
    }
}

int list_is_empty (struct ListNode* list)
{
    return (list->prev == list) && (list->next == list);
}

int list_clear (struct ListNode* list)
{
    struct ListNode* cur = list->next;
    while (cur != list)
    {
        struct ListNode* next = cur->next;
        free (cur);
        cur = next;
    }
    list_init (list);
    return 0;
}

/*
 * Implementation of the binary search tree.
 */

typedef int Data;

struct TreeNode
{
    Data val;
    struct TreeNode *left, *right;
};

struct TreeNode* tree_create_leaf (Data x)
{
    struct TreeNode* leaf = (struct TreeNode*) malloc (sizeof (struct TreeNode));
    leaf->val = x;
    leaf->left = leaf->right = NULL;
    return leaf;
}

struct TreeNode* tree_add (struct TreeNode* tree, Data x)
{
    if (tree) {
        struct TreeNode* node = tree;

        for (;;) {
            if (x == node->val) {
                return tree;
            } else if (x < node->val) {
                if (node->left) {
                    node = node->left;
                } else {
                    node->left = tree_create_leaf (x);
                    return tree;
                }
            } else {
                if (node->right) {
                    node = node->right;
                } else {
                    node->right = tree_create_leaf (x);
                    return tree;
                }
            }
        }
    } else {
        return tree_create_leaf (x);
    }
}

void tree_print (struct TreeNode* tree)
{
    if (tree->left) {
        tree_print (tree->left);
    }
    printf ("%d ", tree->val);
    if (tree->right) {
        tree_print (tree->right);
    }
}

void tree_destroy (struct TreeNode* tree)
{
    if (tree->left) {
        tree_destroy (tree->left);
    }
    if (tree->right) {
        tree_destroy (tree->right);
    }
    free (tree);
}

void tree_breadth_first_search (struct TreeNode* tree, void(*callback) (struct TreeNode*))
{
    struct ListNode queue, *current, *next;
    list_init (&queue);
    current = list_push_back (&queue, tree);

    for (; current != &queue; current = next) {
        struct TreeNode* item = current->data;

        if (item->left) {
            list_push_back (&queue, item->left);
        }
        if (item->right) {
            list_push_back (&queue, item->right);
        }
        callback (item);

        next = current->next;
        list_delete (current);
    }

    assert (list_is_empty (&queue));
    list_clear (&queue);
}

void tree_print_node (struct TreeNode* node)
{
    printf ("%d ", node->val);
}

int main()
{
    struct TreeNode* tree = NULL;

    int x;
    while ((scanf ("%d", &x) == 1) && (x != 0)) {
        tree = tree_add (tree, x);
    }

    tree_breadth_first_search (tree, &tree_print_node);
    putchar ('\n');

    tree_destroy (tree);
}

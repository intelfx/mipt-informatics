#include <stdio.h>
#include <stdlib.h>

typedef int Data;

struct Node
{
    Data val;
    struct Node *left, *right;
};

struct Node* tree_create_leaf (Data x)
{
    struct Node* leaf = (struct Node*) malloc (sizeof (struct Node));
    leaf->val = x;
    leaf->left = leaf->right = NULL;
    return leaf;
}

struct Node* tree_add (struct Node* tree, Data x)
{
    if (tree) {
        struct Node* node = tree;

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

void tree_print (struct Node* tree)
{
    if (tree->left) {
        tree_print (tree->left);
    }
    printf ("%d ", tree->val);
    if (tree->right) {
        tree_print (tree->right);
    }
}

long tree_get_height_and_check_balance (struct Node* tree, long height)
{
    long base = height + 1,
         left_height = tree->left ? tree_get_height_and_check_balance (tree->left,
                                                                       base) : base,
         right_height = tree->right ? tree_get_height_and_check_balance (tree->right,
                                                                         base) : base;

    if (!left_height ||
        !right_height ||
        labs (left_height - right_height) > 1) {
        /* unbalanced */
        return 0;
    } else {
        if (left_height > base) {
            base = left_height;
        }
        if (right_height > base) {
            base = right_height;
        }
        return base;
    }
}

void tree_destroy (struct Node* tree)
{
    if (tree->left) {
        tree_destroy (tree->left);
    }
    if (tree->right) {
        tree_destroy (tree->right);
    }
    free (tree);
}

int main()
{
    struct Node* tree = NULL;

    int x;
    while ((scanf ("%d", &x) == 1) && (x != 0)) {
        tree = tree_add (tree, x);
    }

    printf ("%s\n", tree_get_height_and_check_balance (tree, 0) ? "YES" : "NO");

    tree_destroy (tree);
}

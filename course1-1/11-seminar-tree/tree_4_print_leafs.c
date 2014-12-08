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

void tree_print_leafs (struct Node* tree)
{
    int has_children = 0;
    if (tree->left) {
        tree_print_leafs (tree->left);
        has_children = 1;
    }
    if (tree->right) {
        tree_print_leafs (tree->right);
        has_children = 1;
    }
    if (!has_children) {
        printf ("%d ", tree->val);
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

    tree_print_leafs (tree);
    putchar ('\n');

    tree_destroy (tree);
}

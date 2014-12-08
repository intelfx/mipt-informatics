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

size_t tree_get_height (struct Node* tree, size_t height)
{
    size_t result = height + 1, tmp;

    if (tree->left && (tmp = tree_get_height (tree->left, height + 1)) > result) {
        result = tmp;
    }
    if (tree->right && (tmp = tree_get_height (tree->right, height + 1)) > result) {
        result = tmp;
    }

    return result;
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

    printf ("%zu\n", tree_get_height (tree, 0));

    tree_destroy (tree);
}

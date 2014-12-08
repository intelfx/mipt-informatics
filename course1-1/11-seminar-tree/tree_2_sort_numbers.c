#include <stdio.h>
#include <stdlib.h>

typedef int Data;

struct Node {
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

    int value;
    while ((scanf ("%d", &value) == 1) &&
           (value != 0)) {
        tree = tree_add (tree, value);
    }

    tree_print (tree);
    putchar ('\n');

    tree_destroy (tree);
}

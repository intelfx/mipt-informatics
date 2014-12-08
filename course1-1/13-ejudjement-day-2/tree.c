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

void tree_print (struct Node* tree, int current_height, int height_filter)
{
    if (current_height == height_filter) {
        printf ("%d ", tree->val);
    } else if (current_height < height_filter) {
        if (tree->left) {
            tree_print (tree->left, current_height + 1, height_filter);
        }
        if (tree->right) {
            tree_print (tree->right, current_height + 1, height_filter);
        }
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

    int height;
    scanf ("%d", &height);

    tree_print (tree, 0, height);
    putchar ('\n');

    tree_destroy (tree);
}

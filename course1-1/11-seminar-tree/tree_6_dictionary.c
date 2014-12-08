#include <stdio.h>
#include <stdlib.h>

typedef int Key;

typedef struct {
    size_t count;
} Value;

struct Node
{
    Key key;
    Value value;
    struct Node *left, *right;
};

struct Node* tree_create_leaf (Key k)
{
    struct Node* leaf = (struct Node*) calloc (1, sizeof (struct Node));
    leaf->key = k;
    return leaf;
}

struct Node* tree_add (struct Node* tree, Key k, Value** new)
{
    if (tree) {
        struct Node* node = tree;

        for (;;) {
            if (k == node->key) {
                *new = &node->value;
                return tree;
            } else if (k < node->key) {
                if (node->left) {
                    node = node->left;
                } else {
                    node->left = tree_create_leaf (k);
                    *new = &node->left->value;
                    return tree;
                }
            } else {
                if (node->right) {
                    node = node->right;
                } else {
                    node->right = tree_create_leaf (k);
                    *new = &node->right->value;
                    return tree;
                }
            }
        }
    } else {
        tree = tree_create_leaf (k);
        *new = &tree->value;
        return tree;
    }
}

void tree_print (struct Node* tree)
{
    if (tree->left) {
        tree_print (tree->left);
    }
    printf ("%d %zu\n", tree->key, tree->value.count);
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

    int x;
    while ((scanf ("%d", &x) == 1) && (x != 0)) {
        Value* newly_added = NULL;
        tree = tree_add (tree, x, &newly_added);
        ++newly_added->count;
    }

    tree_print (tree);

    tree_destroy (tree);
}

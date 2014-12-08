#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define BUCKETS 100019
#define HASH(p) ((p).x)

struct Key
{
    int x, y;
};

struct Value
{
    char color;
};

struct Object
{
    struct Key key;
    struct Value value;
};

struct Bucket
{
    struct Object* data;
    size_t n;
    size_t allocated;
};

struct Hashtable
{
    struct Bucket buckets[BUCKETS];
};

struct Object* hash_find_or_add (struct Hashtable* h, struct Key k)
{
    int hash = HASH(k);
    while (hash < 0) {
        hash += BUCKETS;
    }
    struct Bucket* b = &h->buckets[hash % BUCKETS];

    size_t i;
    for (i = 0; i < b->n; ++i) {
        if (!memcmp (&k, &b->data[i].key, sizeof (struct Key))) {
            return &b->data[i];
        }
    }

    if (b->n >= b->allocated) {
        b->allocated = b->allocated ? (2 * b->allocated)
                                    : 1;
        b->data = realloc (b->data, sizeof (struct Object) * b->allocated);
    }

    struct Object* newly_created = &b->data[b->n++];
    newly_created->key = k;
    memset (&newly_created->value, 0, sizeof (struct Value));
    return newly_created;
}

struct Hashtable* hash_create()
{
    return (struct Hashtable*) calloc (1, sizeof (struct Hashtable));
}

void hash_destroy (struct Hashtable* h)
{
    size_t i = 0;

    for (i = 0; i < BUCKETS; ++i) {
        free (h->buckets[i].data);
        h->buckets[i].n = 0;
        h->buckets[i].allocated = 0;
    }

    free (h);
}

int main()
{
    struct Hashtable* h = hash_create();
    int N, i;

    scanf ("%d", &N);

    for (i = 0; i < N; ++i) {
        int type;
        struct Key k;
        scanf ("%d %d %d", &type, &k.x, &k.y);

        struct Object* obj = hash_find_or_add (h, k);

        switch (type) {
        case 1: {
            scanf (" %c", &obj->value.color);
            break;
        }

        case 2: {
            printf ("%c\n", obj->value.color ?: 'N');
            break;
        }

        default:
            abort();
        }
    }

    hash_destroy (h);
    return 0;
}

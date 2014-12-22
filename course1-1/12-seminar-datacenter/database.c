#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <assert.h>

#define BUCKETS 100003
#define HASH(k) (k)

/*
 * The stored item
 */

typedef int Key;

typedef struct {
        int A, B;
} Value;

/*
 * Generic object pool
 */

struct Pool {
        void *data;
        size_t n;
        size_t allocated;
};

typedef size_t PoolIndex;
static const PoolIndex NO_ITEM = (size_t) -1;

void pool_init(struct Pool *pool)
{
        memset(pool, 0, sizeof(*pool));
}

PoolIndex pool_add_raw(struct Pool *pool, size_t size) {
        if (pool->n >= pool->allocated) {
                pool->allocated = 2 * (pool->allocated ?: 1);
                pool->data = realloc(pool->data,
                                        size * pool->allocated);
        }

        return pool->n++;
};
#define pool_add(pool, type) pool_add_raw(pool, sizeof(type))

#define pool_get(pool, type, idx) ((type*)(pool)->data)[idx]

void pool_remove_last(struct Pool *pool)
{
        --pool->n;
}

void pool_destroy(struct Pool *pool)
{
        free(pool->data);
}

/*
 * A hash table with key embedded in the value and external storage.
 */

struct Item {
        Value value;
        size_t count;
};

typedef struct Pool Bucket;
#define bucket_add(bucket) pool_add(bucket, PoolIndex)
#define bucket_destroy(bucket) pool_destroy(bucket);
#define bucket_get(bucket, idx) pool_get(bucket, PoolIndex, idx)

typedef struct Item*(*item_allocator_t)(void);
typedef Key(*key_generator_t)(const Value*);
typedef void(*iterator_t)(const struct Item*, void*);

struct Hashtable {
        Bucket buckets[BUCKETS];
        key_generator_t get_key;
};

Bucket *hash_get_bucket(struct Hashtable *h, Key k)
{
        int hash = HASH(k) % BUCKETS;
        while (hash < 0) {
                hash += BUCKETS;
        }
        while (hash >= BUCKETS) {
                hash -= BUCKETS;
        }
        return &h->buckets[hash];
}

size_t hash_get_count_in_bucket(struct Hashtable *h, const Value *v)
{
        Key k = h->get_key(v);
        Bucket *b = hash_get_bucket(h, k);
        return b->n;
}

PoolIndex *hash_find_add(struct Hashtable *h, struct Pool *pool, const Value *v)
{
        Key k = h->get_key(v);
        Bucket *b = hash_get_bucket(h, k);
        PoolIndex *first_deleted = NULL;

        size_t i;
        for (i = 0; i < b->n; ++i) {
                PoolIndex *index = &bucket_get(b, i); // i-th item in our bucket: index of data global pool
                if (*index == NO_ITEM) {
                        if (!first_deleted) {
                                first_deleted = index;
                        }
                        continue;
                }

                struct Item *item = &pool_get(pool, struct Item, *index); // corresponding data
                if (!item->count) {
                        continue;
                }

                if (!memcmp (v, &item->value, sizeof(*v))) {
                        return index;
                }
        }

        if (first_deleted) {
                return first_deleted;
        } else {
                PoolIndex *place = &bucket_get(b, bucket_add(b));
                *place = NO_ITEM;
                return place;
        }
}

PoolIndex *hash_force_add(struct Hashtable *h, const Value *v)
{
        Key k = h->get_key(v);
        Bucket *b = hash_get_bucket(h, k);

        PoolIndex *place = &bucket_get(b, bucket_add(b));
        *place = NO_ITEM;
        return place;
}

/*
int hash_iterate_by_value(struct Hashtable *h,
                          const Value* v,
                          iterator_t iterator,
                          void* context,
                          int delete)
{
        Key k = h->get_key(v);
        struct Bucket *b = hash_get_bucket(h, k);

        size_t i;
        for (i = 0; i < b->n; ++i) {
                if (!memcmp (v, &b->data[i].value, sizeof(*v))) {
                        int count = b->data[i].count;
                        if (iterator) {
                                iterator(&b->data[i], context);
                        }
                        if (delete) {
                                b->data[i].count = 0;
                        }
                        return count;
                }
        }

        return 0;
}
*/
size_t hash_iterate_by_key(struct Hashtable *h,
                           struct Pool *pool,
                           Key k,
                           iterator_t iterator,
                           void* context,
                           int delete)
{
        Bucket *b = hash_get_bucket(h, k);

        size_t i;
        size_t count = 0;
        for (i = 0; i < b->n; ++i) {
                PoolIndex *index = &bucket_get(b, i); // i-th item in our bucket: index of data global pool
                if (*index == NO_ITEM) {
                        continue;
                }

                struct Item *item = &pool_get(pool, struct Item, *index); // corresponding data
                if (!item->count) {
                        continue;
                }

                if (k == h->get_key(&item->value)) {
                        count += item->count;

                        if (iterator) {
                                iterator(item, context);
                        }

                        if (delete) {
                                /* for this hashtable, for item to be reused */
                                *index = NO_ITEM;
                                /* for other hashtables (it'd be better to get _their_ indices, but well... */
                                item->count = 0;
                        }
                }
        }

        return count;
}


struct Hashtable *hash_create (key_generator_t get_key)
{
        struct Hashtable *h = calloc(1, sizeof(struct Hashtable));
        h->get_key = get_key;
        return h;
}

void hash_destroy(struct Hashtable *h)
{
        size_t i = 0;

        for (i = 0; i < BUCKETS; ++i) {
                bucket_destroy(&h->buckets[i]);
        }

        free(h);
}

int key_A (const Value* v)
{
        return v->A;
}

int key_B (const Value* v)
{
        return v->B;
}

#define MAX_COMMAND 16
/*
typedef struct {
        struct Hashtable *another_hashtable;
        int another_hashtable_count;
} DeleteContext;

void deleter(const struct Object* obj, void* param)
{
        DeleteContext* ctx = param;
        ctx->another_hashtable_count += hash_iterate_by_value(ctx->another_hashtable,
                                                              &obj->value,
                                                              NULL,
                                                              NULL,
                                                              1);
}
*/

/*
 * Picks a smallest hash-table to attempt to insert @v in.
 * Returns NULL if item is guaranteed not to be contained in any of them.
 */
struct Hashtable *pick_smallest(struct Hashtable **tables, Value *v)
{
        struct Hashtable **h, *smallest = NULL;
        size_t bucket_size_in_smallest = (size_t)-1;

        for (h = tables; *h; ++h) {
                size_t bucket_size = hash_get_count_in_bucket(*h, v);

                if (!bucket_size) {
                        return NULL;
                }

                if (bucket_size < bucket_size_in_smallest) {
                        bucket_size_in_smallest = bucket_size;
                        smallest = *h;
                }
        }

        return smallest;
}

/*
 * Adds a given item to all hash-tables except @ignore.
 */
void add_to_all_other(struct Hashtable **tables, struct Hashtable *ignore, Value *value, PoolIndex index)
{
        struct Hashtable **h;

        for (h = tables; *h; ++h) {
                if (*h != ignore) {
                        *hash_force_add(*h, value) = index;
                }
        }
}

/*
 * Adds a given item to all hash-tables.
 */
void add_to_all(struct Hashtable **tables, Value *value, PoolIndex index)
{
        struct Hashtable **h;

        for (h = tables; *h; ++h) {
                *hash_force_add(*h, value) = index;
        }
}

void destroy_all(struct Hashtable **tables)
{
        struct Hashtable **h;

        for (h = tables; *h; ++h) {
                hash_destroy(*h);
        }
}

int main()
{
        struct Hashtable *tables[] = { hash_create(&key_A),
                                       hash_create(&key_B),
                                       NULL };

        struct Pool pool;
        pool_init(&pool);

        int command_count = 0;
        char command[MAX_COMMAND];

        scanf (" %d", &command_count);

        while (command_count--) {
                scanf(" %15s", command);

                if (!strcasecmp(command, "add")) {

                        Value v;
                        scanf (" %d %d", &v.A, &v.B);

                        /* optimal hashtable to find existing item in, or NULL if all are empty */
                        struct Hashtable *h = pick_smallest(tables, &v);
                        PoolIndex *place = h ? hash_find_add(h, &pool, &v) : NULL;
                        struct Item *item;

                        if (place && *place != NO_ITEM) {
                                item = &pool_get(&pool, struct Item, *place);
                                ++item->count;
                        } else {
                                PoolIndex index = pool_add(&pool, struct Item);

                                item = &pool_get(&pool, struct Item, index);
                                item->value = v;
                                item->count = 1;

                                if (h) {
                                        *place = index;
                                        add_to_all_other(tables, h, &v, index);
                                } else {
                                        add_to_all(tables, &v, index);
                                }
                        }

                        printf ("%zu\n", item->count);

                } else if (!strcasecmp(command, "count")) {
                        char field;
                        int value;
                        scanf (" %c %d", &field, &value);

                        size_t count;
                        switch (field) {
                        case 'a':
                        case 'A':
                                count = hash_iterate_by_key(tables[0],
                                                            &pool,
                                                            value,
                                                            NULL,
                                                            NULL,
                                                            0);
                                break;

                        case 'b':
                        case 'B':
                                count = hash_iterate_by_key(tables[1],
                                                            &pool,
                                                            value,
                                                            NULL,
                                                            NULL,
                                                            0);
                                break;

                        default:
                                fprintf (stderr, "Invalid field: '%c'\n", field);
                                abort();
                        }

                        printf ("%zu\n", count);

                } else if (!strcasecmp(command, "delete")) {
                        char field;
                        int value;
                        scanf (" %c %d", &field, &value);

                        size_t count;
                        switch (field) {
                                case 'a':
                                case 'A':
                                        count = hash_iterate_by_key(tables[0],
                                                                    &pool,
                                                                    value,
                                                                    NULL,
                                                                    NULL,
                                                                    1);
                                        break;

                                case 'b':
                                case 'B':
                                        count = hash_iterate_by_key(tables[1],
                                                                    &pool,
                                                                    value,
                                                                    NULL,
                                                                    NULL,
                                                                    1);
                                        break;

                                default:
                                        fprintf (stderr, "Invalid field: '%c'\n", field);
                                        abort();
                        }

                        printf ("%zu\n", count);

                } else {
                        fprintf (stderr, "Invalid command: '%s'\n", command);
                        abort();
                }
        }

        destroy_all(tables);
        pool_destroy(&pool);
        return 0;
}

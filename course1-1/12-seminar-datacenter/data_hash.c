#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define BUCKETS 100003
#define HASH(p) (p)

typedef int Key;
typedef int Value;

struct Object {
	Key key;
	Value value;
};

struct Bucket {
	struct Object *data;
	size_t n;
	size_t allocated;
};

struct Hashtable {
	struct Bucket buckets[BUCKETS];
};

struct Object *hash_find_or_add(struct Hashtable *h, Key k, Value v)
{
	int hash = HASH(k);
	while (hash < 0) {
		hash += BUCKETS;
	}
	struct Bucket *b = &h->buckets[hash % BUCKETS];

	size_t i;
	for (i = 0; i < b->n; ++i) {
		if (!memcmp(&k, &b->data[i].key, sizeof(Key))) {
			return &b->data[i];
		}
	}

	if (b->n >= b->allocated) {
		b->allocated = 2 * (b->allocated ? : 1);
		b->data =
		    realloc(b->data, sizeof(struct Object) * b->allocated);
	}

	struct Object *newly_created = &b->data[b->n++];
	newly_created->key = k;
	newly_created->value = v;
	return newly_created;
}

struct Hashtable *hash_create()
{
	return (struct Hashtable *)calloc(1, sizeof(struct Hashtable));
}

void hash_destroy(struct Hashtable *h)
{
	size_t i = 0;

	for (i = 0; i < BUCKETS; ++i) {
		free(h->buckets[i].data);
		h->buckets[i].n = 0;
		h->buckets[i].allocated = 0;
	}

	free(h);
}

int main()
{
	struct Hashtable *h = hash_create();
	int value, r;
	struct Object *obj = NULL;

	while ((r = scanf("%d", &value)) != -1) {
		/* obj holds pointer to the pair associated with the previous
		 * read value, or NULL. */
		if (obj) {
			obj->value = value;
		}

		/* find the pair associated with current value. */
		obj = hash_find_or_add(h, value, -1);
	}

	printf("%d\n", obj->value);

	hash_destroy(h);
	return 0;
}

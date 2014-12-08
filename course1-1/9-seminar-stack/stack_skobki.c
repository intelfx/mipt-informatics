#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

typedef char Data;

struct Stack {
    int n;
    int size;
    Data * a;
};

struct Stack * stack_create(int size)
{
    struct Stack * s = malloc (sizeof (struct Stack));
    s->n = 0;
    s->size = size;
    s->a = malloc (sizeof (*s->a) * size);
    return s;
}

int stack_is_empty(struct Stack * s)
{
    return !s->n;
}

void stack_push(struct Stack * s, Data x)
{
    /* reallocate if needed */
    while (s->n >= s->size) {
        s->a = realloc (s->a, sizeof (*s->a) * (s->size *= 2));
    }
    s->a[s->n++] = x;
}

Data stack_pop(struct Stack * s)
{
    assert (s->n);
    return s->a[--s->n];
}

Data stack_get(struct Stack * s)
{
    assert (s->n);
    return s->a[s->n - 1];
}

void stack_print(struct Stack * s)
{
    if (s->n) {
        for (int i = 0; i < s->n; ++i) {
            printf ("%d ", s->a[i]);
        }
        putchar ('\n');
    } else {
        printf ("Empty stack\n");
    }
}

int stack_size(struct Stack * s)
{
    return s->n;
}

void stack_clear(struct Stack * s)
{
    s->n = 0;
}

void stack_destroy(struct Stack * s)
{
    free (s->a);
    free (s);
}

int main()
{
    char* str = 0;
    int str_len = 0;
    int failure = 0;
    struct Stack* s;

    scanf ("%m[^\n]%n", &str, &str_len);
    s = stack_create(str_len / 2);

    for (int i = 0; !failure && (i < str_len); ++i) {
        switch (str[i]) {
#define HANDLE_BRACKET_PAIR(opening, closing)                                  \
    case opening: stack_push (s, opening); break;                          \
    case closing: if (stack_is_empty (s) || (stack_pop (s) != opening))    \
        { failure = 1; } break;

        HANDLE_BRACKET_PAIR ('(', ')')
        HANDLE_BRACKET_PAIR ('[', ']')
        HANDLE_BRACKET_PAIR ('{', '}')
        HANDLE_BRACKET_PAIR ('<', '>')

#undef HANDLE_BRACKET_PAIR
        }
    }

    if (!stack_is_empty (s)) {
        failure = 1;
    }

    printf ("%s\n", failure ? "NO" : "YES");

    stack_destroy (s);
    free (str);
}

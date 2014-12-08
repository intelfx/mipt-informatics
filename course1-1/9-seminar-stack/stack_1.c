#include <stdio.h>

void stack_create(struct Stack * s)
{
    s->n = 0;
}

void stack_push(struct Stack * s, Data x)
{
    s->a[s->n++] = x;
}

Data stack_pop(struct Stack * s)
{
    return s->a[--s->n];
}

Data stack_get(struct Stack * s)
{
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

int  stack_size(struct Stack * s)
{
    return s->n;
}

void stack_clear(struct Stack * s)
{
    s->n = 0;
}

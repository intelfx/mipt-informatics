#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

typedef int Data;

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
    struct Stack* s = stack_create (2);
    int end = 0;

    while (!end) {
        int value;
        char op[2];

        if (scanf (" %1[+-/*=] ", op) == 1) {
            if (op[0] == '=') {
                break;
            } else {
                int o1 = stack_pop (s),
                    o2 = stack_pop (s);

                switch (op[0]) {
                case '+':
                    stack_push (s, o2 + o1);
                    break;

                case '-':
                    stack_push (s, o2 - o1);
                    break;

                case '*':
                    stack_push (s, o2 * o1);
                    break;

                case '/':
                    stack_push (s, o2 / o1);
                    break;

                default:
                    abort();
                }
            }
        } else if (scanf (" %d ", &value) == 1) {
            stack_push (s, value);
        } else {
            abort();
        }
    }

    assert (stack_size (s) == 1);
    printf ("%d\n", stack_pop (s));
    stack_destroy (s);
}

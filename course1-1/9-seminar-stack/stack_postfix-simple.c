#include <stdio.h>
#include <stdlib.h>

static unsigned BUFFER_SIZE = 1024;

void check_stack_top (int top)
{
    if (top < 0) {
        printf ("ERROR: stack underflow\n");
        exit (1);
    }
}

int main()
{
    int stack[BUFFER_SIZE];
    int stack_top = -1;

    for (;;) {
        int value;
        char op[2];

        if (scanf (" %1[+-/*=]", op) == 1) {
            if (op[0] == '=') {
                break;
            } else {
                check_stack_top (stack_top);
                int o1 = stack[stack_top--];

                check_stack_top (stack_top);
                int o2 = stack[stack_top--];

                switch (op[0]) {
                case '+':
                    stack[++stack_top] = o2 + o1;
                    break;

                case '-':
                    stack[++stack_top] = o2 - o1;
                    break;

                case '*':
                    stack[++stack_top] = o2 * o1;
                    break;

                case '/':
                    stack[++stack_top] = o2 / o1;
                    break;

                default:
                    abort();
                }
            }
        } else if (scanf (" %d ", &value) == 1) {
            stack[++stack_top] = value;
        } else {
            abort();
        }
    }

    printf ("%d\n", stack[stack_top]);
    return 0;
}

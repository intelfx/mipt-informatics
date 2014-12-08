#include <stdio.h>
#include <stdlib.h>

int main()
{
    int a, b, result;
    char opbuf[2];

    scanf ("%d %1[v^] %d", &a, opbuf, &b);

    switch (opbuf[0]) {
    case 'v': result = a || b; break;
    case '^': result = a && b; break;
    default: abort();
    }

    printf ("%d\n", result);
}

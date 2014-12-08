#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

int main()
{
    int depth = 0;
    int c;

    while ((c = getchar()) != EOF && depth >= 0) {
        if (isspace (c)) {
            continue;
        }

        switch (c) {
        case '(':
            ++depth;
            break;

        case ')':
            --depth;
            break;

        default:
            abort();
        }
    }

    if (depth != 0) {
        printf ("NO\n");
    } else {
        printf ("YES\n");
    }
}

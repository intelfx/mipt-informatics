#include <string.h>
#include <stdlib.h>
#include <ctype.h>

char* convert (const char* str)
{
    size_t len = strlen (str);
    char *ret = (char*) malloc (len + 1),
         *cur = ret;

    while (*str) {
        if (!isspace (*str)) {
            *cur++ = *str++;
        } else {
            ++str;
        }
    }
    *cur = 0;

    return ret;
}

#include <stdio.h>

int main()
{
    char* in = NULL;

    scanf ("%m[^.]", &in);
    char* out = convert (in);

    printf ("'%s'\n", out);

    free (in);
    free (out);
}

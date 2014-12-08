#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int cmp_char (const void* p1, const void* p2)
{
    int r = *(const char*)p1 - *(const char*)p2;
    if (r < 0) return -1;
    else if (r > 0) return 1;
    else return 0;
}

int main()
{
    char* data = 0;
    scanf ("%m[^.]", &data);

    qsort (data, strlen (data), 1, cmp_char);

    printf ("%s.\n", data);
    free (data);
}

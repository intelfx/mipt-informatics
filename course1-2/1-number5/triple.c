#include <stdio.h>
#include <stdlib.h>

int main()
{
        char* value_tri = 0;
        scanf ("%ms", &value_tri);

        long value = strtol (value_tri, 0, 3);
        printf ("%ld\n", value);

        free (value_tri);
        return 0;
}
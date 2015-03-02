#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

int main()
{
        int radix_in, radix_out;
        char* value_in = 0;

        scanf ("%d", &radix_in);
        scanf ("%ms", &value_in);
        scanf ("%d", &radix_out);

        unsigned long value = strtoul (value_in, 0, radix_in);

        char value_out[32], *value_out_ptr = value_out + sizeof (value_out);

        *--value_out_ptr = 0;
        while (value) {
                assert (value_out_ptr != value_out);
                *--value_out_ptr = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"[value % radix_out];
                value /= radix_out;
        }

        if (*value_out_ptr) {
                printf ("%s\n", value_out_ptr);
        } else {
                printf ("0\n");
        }

        free (value_in);
        return 0;
}
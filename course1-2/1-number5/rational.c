#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

int main()
{
        char *value_in = 0, *value_in_fractional_part;

        scanf ("%ms", &value_in);

        /* integral part */
        double value_out = strtol (value_in, &value_in_fractional_part, 2);

        /* fractional part */
        if (*value_in_fractional_part) {
                assert (*value_in_fractional_part == '.');
                ++value_in_fractional_part;

                long numerator = strtol (value_in_fractional_part, 0, 2);
                long denominator_log2 = strlen (value_in_fractional_part);

                value_out += (double) numerator / (1 << denominator_log2);
        }

        printf ("%.12lf\n", value_out);

        free (value_in);
        return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

unsigned read_rome_char (char c)
{
        switch (toupper (c)) {
        case 'I': return 1;
        case 'V': return 5;
        case 'X': return 10;
        case 'L': return 50;
        case 'C': return 100;
        case 'D': return 500;
        case 'M': return 1000;
        default:
                fprintf (stderr, "offending character: '%c'\n", c);
                abort();
        }
}

int main()
{
        char *value_in = 0, *value_in_ptr;

        scanf ("%ms", &value_in);
        value_in_ptr = value_in + strlen (value_in);

        unsigned long value = 0;
        unsigned last_digit = 0;

        do {
                --value_in_ptr;

                unsigned digit = read_rome_char (*value_in_ptr);
                if (digit < last_digit) {
                        value -= digit;
                } else {
                        value += digit;
                }

                last_digit = digit;
        } while (value_in_ptr != value_in);

        printf ("%lu\n", value);

        free (value_in);
        return 0;
}
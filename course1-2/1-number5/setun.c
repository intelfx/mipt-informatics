#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int read_setun_char (char c)
{
        switch (c) {
        case '0':
                return 0;

        case '1':
                return 1;

        case '$':
                return -1;

        default:
                fprintf (stderr, "offending character: '%c'\n", c);
                abort();
        }
}

char write_setun_char (int value)
{
        switch (value) {
        case 0:
                return '0';

        case 1:
                return '1';

        case -1:
                return '$';

        default:
                fprintf (stderr, "offending value: %d\n", value);
                abort();
        }
}

#define MAX(a, b) (((a) > (b)) ? (a) : (b))

char* pad_front (char* string, char padding, size_t old_length, size_t new_length)
{
        char* result = realloc (string, new_length);
        memmove (result + new_length - old_length, result, old_length);
        memset (result, padding, new_length - old_length);

        return result;
}

void sum_setun (char* a, char* b, size_t length)
{
        size_t i = length;
        int carry = 0;
        do {
                --i;

                int sum = carry + read_setun_char(a[i]) + read_setun_char(b[i]);
                carry = 0;

                while (sum > 1) {
                        sum -= 3;
                        ++carry;
                }

                while (sum < -1) {
                        sum += 3;
                        --carry;
                }

                a[i] = write_setun_char (sum);
        } while (i);
}

char* skip_padding (char* string)
{
        char* ret = string;

        while (*ret == '0' && ret[1]) {
                ++ret;
        }
        return ret;
}

int main()
{
        char *a = 0, *b = 0;
        scanf ("%ms", &a);
        scanf ("%ms", &b);

        size_t a_length = strlen (a),
               b_length = strlen (b),
               length = MAX (a_length, b_length) + 1;

        a = pad_front (a, '0', a_length, length);
        b = pad_front (b, '0', b_length, length);

        sum_setun (a, b, length);

        printf ("%s\n", skip_padding (a));

        return 0;
}
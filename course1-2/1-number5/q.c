#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned long long data_t;

#define TO_RADIX(radix, a, b, c) ((a) * (radix) * (radix) + (b) * (radix) + (c))

int main()
{
        unsigned radix;
        scanf ("%u", &radix);

        data_t min = radix * radix, /* 100 */
               max = radix * radix * radix; /* 1000 */

        data_t result = 0;

        data_t tail_check_value = radix * radix * radix; /* 1000 -- will div.by 1000 if three last digits are zero */

        for (data_t i = min; i < max; ++i) {
                data_t sq = i * i;

                if (!((sq - i) % (tail_check_value))) {
                        result = i;
                }
        }

        printf ("%llu\n", result);

        return 0;
}
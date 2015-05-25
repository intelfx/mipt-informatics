#include <stdio.h>

static int integer_array[] = { 1, 2, 3, -5 };
static double fp_array[]   = { 0.5, 2, 3.5, -7 };

extern int sum1 (const int array[], size_t count);
extern double sum2 (const double array[], size_t count);

int main()
{
        printf ("sum of integers: %d\n",
                sum1 (integer_array,
                      sizeof (integer_array) / sizeof (*integer_array)));

        printf ("sum of double-precision FPs: %g\n",
                sum2 (fp_array,
                      sizeof (fp_array) / sizeof (*fp_array)));

        return 0;
}
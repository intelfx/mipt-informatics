#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int cmp_double (const void* p1, const void* p2)
{
    double r = *(const double*)p1 - *(const double*)p2;
    if (r < 0) return -1;
    else if (r > 0) return 1;
    else return 0;
}

int main()
{
    size_t N;
    scanf ("%zu ", &N);

    double* data = (double*) malloc (sizeof (double) * N);
    for (size_t i = 0; i < N; ++i) {
        scanf ("%lf ", data + i);
    }

    qsort (data, N, sizeof (*data), cmp_double);

    for (size_t i = 0; i < N; ++i) {
        printf ("%lf ", data[i]);
    }
    printf ("\n");

    free (data);
}

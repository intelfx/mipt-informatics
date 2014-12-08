#include <stdio.h>
#include <stdlib.h>

int cmp_int (const void* p1, const void* p2)
{
    return *(const int*)p2 - *(const int*)p1;
}

int main()
{
    size_t N;
    scanf ("%zu ", &N);

    int* data = (int*) malloc (sizeof (int) * N);
    for (size_t i = 0; i < N; ++i) {
        scanf ("%d ", data + i);
    }

    qsort (data, N, sizeof (*data), cmp_int);

    for (size_t i = 0; i < N; ++i) {
        printf ("%d ", data[i]);
    }
    printf ("\n");

    free (data);
}

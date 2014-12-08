#include <stdio.h>
#include <stdlib.h>

int cmp_digits (const void* p1, const void* p2)
{
    int _1 = *(const int*)p1,
        _2 = *(const int*)p2;

    while (_1 || _2) {
        if ((_1 % 10) < (_2 % 10)) {
            return -1;
        } else if ((_1 % 10) > (_2 % 10)) {
            return 1;
        } else {
            _1 /= 10;
            _2 /= 10;
        }
    }

    return 0;
}

int main()
{
    size_t N;
    scanf ("%zu ", &N);

    int* data = (int*) malloc (sizeof (int) * N);
    for (size_t i = 0; i < N; ++i) {
        scanf ("%d ", data + i);
    }

    qsort (data, N, sizeof (*data), cmp_digits);

    for (size_t i = 0; i < N; ++i) {
        printf ("%d ", data[i]);
    }
    printf ("\n");

    free (data);
}

#include <stdio.h>
#include <stdlib.h>

int cmp_int (const void* p1, const void* p2)
{
    return *(const int*)p1 - *(const int*)p2;
}

enum {
    FLAG_STATIC,
    FLAG_SORTED
};

int main()
{
    size_t N, sorted_count = 0;
    scanf ("%zu ", &N);

    int *static_data = (int*) malloc (sizeof (int) * N),
        *sorted_data = (int*) malloc (sizeof (int) * N),
        *flags = (int*) malloc (sizeof (int) * N);
    for (size_t i = 0; i < N; ++i) {
        int val;
        scanf ("%d ", &val);

        if (val % 2) {
            static_data[i] = val;
            flags[i] = FLAG_STATIC;
        } else {
            sorted_data[sorted_count++] = val;
            flags[i] = FLAG_SORTED;
        }
    }

    qsort (sorted_data, sorted_count, sizeof (*sorted_data), cmp_int);

    size_t sorted_i = 0;
    for (size_t i = 0; i < N; ++i) {
        switch (flags[i]) {
            case FLAG_STATIC:
                printf ("%d ", static_data[i]);
                break;

            case FLAG_SORTED:
                printf ("%d ", sorted_data[sorted_i++]);
                break;

            default:
                abort();
        }
    }
    printf ("\n");

    free (static_data);
    free (sorted_data);
    free (flags);
}

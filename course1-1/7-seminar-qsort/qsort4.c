#include <stdio.h>
#include <stdlib.h>
#include <math.h>

struct Point
{
    int x, y;
};

double dist (const void* p)
{
    int x = ((const struct Point*)p)->x,
        y = ((const struct Point*)p)->y;
    return sqrt (x*x + y*y);
}

int cmp_int (const void* p1, const void* p2)
{
    return *(const int*)p1 - *(const int*)p2;
}

int cmp_Point (const void* p1, const void* p2)
{
    double dist1 = dist (p1),
           dist2 = dist (p2);
    int r;
    if (dist1 < dist2) {
        return -1;
    } else if (dist2 < dist1) {
        return 1;
    } else if ((r = cmp_int (&((const struct Point*)p1)->x,
                             &((const struct Point*)p2)->x))) {
        return r;
    } else if ((r = cmp_int (&((const struct Point*)p1)->y,
                             &((const struct Point*)p2)->y))) {
        return r;
    } else {
        return 0;
    }
}

int main()
{
    size_t N;
    scanf ("%zu ", &N);

    struct Point* data = (struct Point*) malloc (sizeof (struct Point) * N);
    for (size_t i = 0; i < N; ++i) {
        scanf ("%d %d ", &data[i].x, &data[i].y);
    }

    qsort (data, N, sizeof (*data), cmp_Point);

    for (size_t i = 0; i < N; ++i) {
        printf ("%d %d\n", data[i].x, data[i].y);
    }

    free (data);
}

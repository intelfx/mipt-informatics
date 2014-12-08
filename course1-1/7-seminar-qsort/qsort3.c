#include <math.h>

double dist (const void* p)
{
    int x = ((const struct Point*)p)->x,
        y = ((const struct Point*)p)->y;
    return sqrt (x*x + y*y);
}

int cmp_Point (const void* p1, const void* p2)
{
    double dist1 = dist (p1),
           dist2 = dist (p2);
    if (dist1 < dist2) {
        return -1;
    } else if (dist2 < dist1) {
        return 1;
    } else {
        return 0;
    }
}

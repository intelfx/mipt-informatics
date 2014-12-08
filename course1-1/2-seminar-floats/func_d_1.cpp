#include <cmath>

double dist (double x1, double y1, double x2, double y2)
{
    double xd = x1 - x2,
           yd = y1 - y2;
    return sqrt (xd*xd + yd*yd);
}

double perimeter (double x1, double y1, double x2, double y2, double x3, double y3)
{
    return dist (x1, y1, x2, y2) +
           dist (x2, y2, x3, y3) +
           dist (x3, y3, x1, y1);
}

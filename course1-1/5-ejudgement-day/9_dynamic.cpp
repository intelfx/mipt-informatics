#include <iostream>
#include <iomanip>

unsigned long *data;
int X, Y;

unsigned long& ac (int x, int y)
{
    return data[y * X + x];
}


int main()
{
    std::cin >> X >> Y;
    ++X;
    ++Y;

    data = new unsigned long[X * Y];

    for (int y = 0; y < Y; ++y) {
        ac (0, y) = 1;
    }

    for (int x = 0; x < X; ++x) {
        ac (x, 0) = 1;
    }

    for (int y = 1; y < Y; ++y) {
        for (int x = 1; x < X; ++x) {
            ac (x, y) = ac (x - 1, y) + ac (x, y - 1);
        }
    }

    std::cout << ac (X - 1, Y - 1) << std::endl;

    delete[] data;
}

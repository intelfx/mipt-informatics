#include <fstream>

int main()
{
    std::ifstream in ("vhod");
    std::ofstream out ("vyhod");
    double x, y;
    in >> x >> y;

    if ((x >= -3) &&
        (x <= 2) &&
        (y >= -4) &&
        (y <= 6)) {
        out << "YES" << std::endl;
    } else {
        out << "NO" << std::endl;
    }

    return 0;
}

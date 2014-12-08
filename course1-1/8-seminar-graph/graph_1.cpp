#include <iostream>

int main()
{
    int vertices, edges = 0;
    std::cin >> vertices;

    for (int i = 0; i < vertices; ++i) {
        for (int j = 0; j < vertices; ++j) {
            int has_edge;
            std::cin >> has_edge;

            if (i < j && has_edge) {
                ++edges;
            }
        }
    }

    std::cout << edges << std::endl;
}

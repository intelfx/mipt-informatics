#include <iostream>
#include <set>
#include <cstdlib>

int* adjacency_matrix;
int vertices;

int& ac (int x, int y)
{
    return adjacency_matrix[x * vertices + y];
}

int main()
{
    int edges;
    std::cin >> vertices >> edges;
    adjacency_matrix = reinterpret_cast<int*> (calloc (vertices * vertices, sizeof (*adjacency_matrix)));

    while (!std::cin.eof()) {
        int v1, v2;
        std::cin >> v1 >> v2 >> std::ws;

        ac (v1, v2) = 1;
        ac (v2, v1) = 1;
    }

    for (int i = 0; i < vertices; ++i) {
        for (int j = 0; j < vertices; ++j) {
            std::cout << ac (i, j) << " ";
        }
        std::cout << std::endl;
    }
}

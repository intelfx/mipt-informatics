#include <iostream>

bool triangle_exists_partial (int edges[3], int base, int side1, int side2)
{
    return edges[base] < edges[side1] + edges[side2];
}

bool triangle_exists (int edges[3])
{
    return triangle_exists_partial (edges, 0, 1, 2) &&
           triangle_exists_partial (edges, 1, 0, 2) &&
           triangle_exists_partial (edges, 2, 0, 1);
}

int main()
{
    int edges[3];
    std::cin >> edges[0] >> edges[1] >> edges[2];
    std::cout << (triangle_exists(edges) ? "YES" : "NO") << std::endl;
}

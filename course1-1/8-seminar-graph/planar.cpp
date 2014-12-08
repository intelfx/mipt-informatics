#include <iostream>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/boyer_myrvold_planar_test.hpp>

using namespace boost;

typedef adjacency_list<vecS,
                       vecS,
                       undirectedS,
                       property<vertex_index_t, int>
                      > Graph;

int main()
{
    int vertices, edges;
    std::cin >> vertices >> edges;

    Graph subject (vertices);
    while (!std::cin.eof()) {
        int e1, e2;
        std::cin >> e1 >> e2 >> std::ws;
        add_edge (e1, e2, subject);
    }

    std::cout << (boyer_myrvold_planarity_test (subject) ? "YES" : "NO")
              << std::endl;
}

#include <iostream>
#include <vector>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/sequential_vertex_coloring.hpp>

using namespace boost;

typedef adjacency_list<listS,
                       vecS,
                       undirectedS,
                       property<vertex_color_t, int>
                      > Graph;
typedef graph_traits<Graph>::vertices_size_type vertices_size_type;

int main()
{
    int vertices, edges;
    std::cin >> vertices >> edges;

    if (!edges) {
        // work-around an apparent bug in boost

        // "no edges" means no placement limitations, so the first table
        // gets all people.
        std::cout << "YES" << std::endl;
        for (int i = 0; i < vertices; ++i) {
            std::cout << i << " ";
        }
        std::cout << std::endl;
        return 0;
    }

    Graph placement (vertices);
    while (!std::cin.eof()) {
        int v1, v2;
        std::cin >> v1 >> v2 >> std::ws;
        add_edge (v1, v2, placement);
    }

    property_map<Graph, vertex_color_t>::type colors = get (vertex_color_t(), placement);
    vertices_size_type num_colors = sequential_vertex_coloring (placement, colors);

    if (num_colors <= 2) {
        std::cout << "YES" << std::endl;
        for (int i = 0; i < vertices; ++i) {
            if (colors[i] == 0) {
                std::cout << i << " ";
            }
        }
        std::cout << std::endl;
    } else {
        std::cout << "NO" << std::endl;
    }
}

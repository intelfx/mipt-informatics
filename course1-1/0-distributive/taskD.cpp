#include <iostream>
#include <vector>
#include <limits>
#include <algorithm>
#include <utility>

struct Dijkstra
{
    unsigned answer;
    bool marked;

    Dijkstra()
        : answer (std::numeric_limits<unsigned>::max())
        , marked (false)
    { }
};

struct Edge
{
    struct Vertex* target;
    unsigned length;
};

struct Vertex
{
    unsigned index;
    std::vector<Edge> edges;

    Dijkstra d;
};

int main()
{
    unsigned vertices, edges;
    std::cin >> vertices >> edges;

    std::vector<Vertex> graph (vertices);

    for (unsigned i = 0; i < edges; ++i) {
        unsigned a, b, l;
        std::cin >> a >> b >> l;
        graph[a].edges.push_back (Edge { &graph[b], l });
        graph[b].edges.push_back (Edge { &graph[a], l });
    }

    unsigned target_vertex;
    std::cin >> target_vertex;
    graph[target_vertex].d.answer = 0;

    for (unsigned iter = 0; iter < vertices; ++iter) {
        Vertex* current_vertex = nullptr;

        // pick vertex
        for (Vertex& vertex: graph) {
            if (!vertex.d.marked) {
                if (!current_vertex || (vertex.d.answer < current_vertex->d.answer)) {
                    current_vertex = &vertex;
                }
            }
        }

        // check if doing more work is pointless
        if (current_vertex->d.answer == std::numeric_limits<unsigned>::max()) {
            break;
        }

        // mark picked vertex
        current_vertex->d.marked = true;

        // relax pathes
        for (const Edge& edge: current_vertex->edges) {
            edge.target->d.answer = std::min (edge.target->d.answer,
                                              current_vertex->d.answer + edge.length);
        }
    }

    // write answer
    for (Vertex& vertex: graph) {
        std::cout << vertex.d.answer << " ";
    }
    std::cout << std::endl;

    return 0;
}

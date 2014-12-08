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

    Edge (struct Vertex* t, unsigned l)
    : target (t)
    , length (l)
    { }
};

typedef std::vector<Edge> EdgeVec;

struct Vertex
{
    unsigned index;
    EdgeVec edges;

    Dijkstra d;
};

typedef std::vector<Vertex> Graph;

int main()
{
    unsigned vertices, edges;
    std::cin >> vertices >> edges;

    Graph graph (vertices);

    for (unsigned i = 0; i < edges; ++i) {
        unsigned a, b, l;
        std::cin >> a >> b >> l;
        graph[a].edges.push_back (Edge ( &graph[b], l ));
        graph[b].edges.push_back (Edge ( &graph[a], l ));
    }

    unsigned target_vertex;
    std::cin >> target_vertex;
    graph[target_vertex].d.answer = 0;

    for (unsigned iter = 0; iter < vertices; ++iter) {
        Vertex* current_vertex = 0;

        // pick vertex
        for (Graph::iterator it = graph.begin(); it != graph.end(); ++it) {
            if (!it->d.marked) {
                if (!current_vertex || (it->d.answer < current_vertex->d.answer)) {
                    current_vertex = &*it;
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
        for (EdgeVec::iterator it = current_vertex->edges.begin();
             it != current_vertex->edges.end();
             ++it) {
            it->target->d.answer = std::min (it->target->d.answer,
                                             current_vertex->d.answer + it->length);
        }
    }

    // write answer
    for (Graph::iterator it = graph.begin(); it != graph.end(); ++it) {
        std::cout << it->d.answer << " ";
    }
    std::cout << std::endl;

    return 0;
}

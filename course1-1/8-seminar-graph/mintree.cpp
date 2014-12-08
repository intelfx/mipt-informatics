#include <iostream>
#include <iterator>
#include <list>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/adjacency_matrix.hpp>
#include <boost/graph/prim_minimum_spanning_tree.hpp>
#include <boost/graph/kruskal_min_spanning_tree.hpp>

using namespace boost;

#undef ADJ_LIST
#define ADJ_MATRIX
#undef MAXTEST
#define PRIM
#undef KRUSKAL

#ifdef ADJ_LIST
typedef adjacency_list<setS,
                       vecS,
                       undirectedS,
                       property<vertex_distance_t, int>,
                       property<edge_weight_t, int>
                      > Graph;
#endif // ADJ_LIST

#ifdef ADJ_MATRIX
typedef adjacency_matrix<undirectedS,
                         property<vertex_distance_t, int>,
                         property<edge_weight_t, int>
                        > Graph;
#endif // ADJ_MATRIX

typedef property_map<Graph, edge_weight_t>::type WeightMap;

int main()
{
    int vertices, edges;
    std::cin >> vertices >> edges;

    if (!vertices || !edges) {
        // just in case
        std::cout << 0 << std::endl;
        return 0;
    }

    Graph subject (vertices);
    WeightMap weights = get (edge_weight_t(), subject);

#ifndef MAXTEST
    while (!std::cin.eof()) {
        int v1, v2, weight;
        std::cin >> v1 >> v2 >> weight >> std::ws;

        if (true) {
            std::pair<graph_traits<Graph>::edge_descriptor, bool> result =
                add_edge (v1, v2, subject);

            if (result.second) {
                weights[result.first] = weight;
            } else {
                weights[result.first] = std::min (weight, weights[result.first]);
            }

        }
    }
#else // MAXTEST
    for (int v1 = 0; v1 < vertices; ++v1) {
        for (int v2 = v1 + 1; v2 < vertices; ++v2) {
            std::pair<graph_traits<Graph>::edge_descriptor, bool> result =
                add_edge (v1, v2, subject);

            if (result.second) {
                weights[result.first] = 256;
            } else {
                weights[result.first] = std::min (256, weights[result.first]);
            }
        }
    }
#endif // MAXTEST

    int total_weight = 0;

#ifdef PRIM
    std::vector<graph_traits<Graph>::edge_descriptor> result;
    kruskal_minimum_spanning_tree (subject, std::back_inserter (result));

    for (size_t i = 0; i < result.size(); ++i) {
        total_weight += weights[result[i]];
        std::cerr << "edge: from=" << source (result[i], subject) << " to=" << target(result[i], subject) << " weight=" << weights[result[i]] << std::endl;
    }
#endif // PRIM

#ifdef KRUSKAL
    std::vector<graph_traits<Graph>::vertex_descriptor> result (vertices);
    prim_minimum_spanning_tree(subject, &result.front());

    for (size_t i = 0; i < result.size(); ++i) {
        if (result[i] != i) {
            std::pair<graph_traits<Graph>::edge_descriptor, bool> e =
                edge (i, result[i], subject);
            if (!e.second) {
                throw std::logic_error ("edge does not exist");
            }
            total_weight += weights[e.first];
        }
    }
#endif // KRUSKAL

    std::cout << total_weight << std::endl;
}

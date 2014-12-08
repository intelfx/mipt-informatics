#include <iostream>
#include <set>

int main()
{
    int vertices = 0;
    std::cin >> vertices;

    typedef std::set< std::pair<int, int> > EdgeSet;
    EdgeSet edges;

    for (int i = 0; i < vertices; ++i) {
        for (int j = 0; j < vertices; ++j) {
            int has_edge;
            std::cin >> has_edge;

            if (i < j && has_edge) {
                edges.insert (std::make_pair (j, i)); // i < j
            }
        }
    }

    for (EdgeSet::iterator it = edges.begin(); it != edges.end(); ++it) {
        std::cout << it->first << " " << it->second << std::endl;
    }
}

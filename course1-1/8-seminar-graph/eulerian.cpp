#include <iostream>

int main()
{
    int vertices;
    std::cin >> vertices;

    bool has_euler_path = true;
    for (int i = 0; i < vertices; ++i) {
        int edges_for_current_vertex = 0;
        for (int j = 0; j < vertices; ++j) {
            int edge_count;
            std::cin >> edge_count;
            if ((i != j) && edge_count) {
                // i != j: loops not counted
                edges_for_current_vertex += edge_count;
            }
        }

        if (edges_for_current_vertex % 2) {
            has_euler_path = false;
            break;
        }
    }

    std::cout << (has_euler_path ? "YES" : "NO") << std::endl;
}

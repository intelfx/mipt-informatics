#include <iostream>

long long edges_in_complete_graph (long long vertices)
{
    return vertices * (vertices - 1) / 2;
}

int main()
{
    int N;
    std::cin >> N;

    for (int i = 0; i < N; ++i) {
        int vertices;
        std::cin >> vertices;

        long long edges = edges_in_complete_graph (vertices - 1) + 1;
        std::cout << edges << " ";
        if (edges == 1) {
            std::cout << "Alice" << std::endl;
        } else {
            std::cout << ((edges % 2) ? "Alice" : "Bob") << std::endl;
        }
    }
}

#include <iostream>

bool can_break (int n, int m, int k)
{
    return (n * m > k) && (!(k % n) || !(k % m));
}

int main()
{
    int n, m, k;
    std::cin >> n >> m >> k;
    std::cout << (can_break (n, m, k) ? "YES" : "NO") << std::endl;
}

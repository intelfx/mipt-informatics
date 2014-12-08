#include <iostream>

int gcd (int a, int b)
{
    while (b != 0)
    {
        int temp = a % b;
        a = b;
        b = temp;
    }
    return a;
}

int main()
{
    int a, b;
    std::cin >> a >> b;

    int g = gcd (a, b);
    std::cout << g << " " << a * b / g << std::endl;
}

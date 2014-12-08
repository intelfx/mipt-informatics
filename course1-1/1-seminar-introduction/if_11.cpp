#include <iostream>

const char* word_by_count (int n)
{
    if (n >= 10 && n <= 20)
        return "korov";

    switch (n % 10) {
    case 1:
        return "korova";

    case 2:
    case 3:
    case 4:
        return "korovy";

    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 0:
        return "korov";
    }
    return 0;
}

int main()
{
    int n;
    std::cin >> n;
    std::cout << n << " " << word_by_count(n) << std::endl;
}

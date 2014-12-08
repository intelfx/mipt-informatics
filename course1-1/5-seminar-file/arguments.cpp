#include <cstdlib>

int main (int argc, char** argv)
{
    int result = 0;
    for (int i = 1; i < argc; ++i) {
        result += std::atoi (argv[i]);
    }
    return result;
}

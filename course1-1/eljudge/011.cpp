#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <vector>

int main()
{
    bool broken = false;
    int c;
    std::vector<char> stack;

    while ((c = getchar()) != EOF) {
        if (isspace (c)) {
            continue;
        }

        int check = 0;

        switch (c) {
        case '(':
        case '[':
        case '{':
        case '<':
            stack.push_back (c);
            break;

        case ')': check = '('; break;
        case ']': check = '['; break;
        case '}': check = '{'; break;
        case '>': check = '<'; break;

        default:
            abort();
        }

        if (check) {
            if (stack.empty()) {
                broken = true;
                break;
            } else if (stack.back() == check) {
                stack.pop_back();
            } else {
                break;
            }
        }
    }

    if (!broken && stack.empty()) {
        printf ("YES\n");
    } else {
        printf ("NO\n");
    }
}

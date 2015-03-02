#include <iostream>

void reduce_string (std::string& str)
{
        std::string::iterator it = str.begin();
        while (*it == '0' && (it + 1 != str.end())) {
                ++it;
        }

        str.erase (str.begin(), it);
}

int main()
{
        std::string a, b;
        std::cin >> a >> b;

        reduce_string (a);
        reduce_string (b);

        a.insert (0, std::max (a.size(), b.size()) - a.size() + 1, '0');
        b.insert (0, std::max (a.size(), b.size()) - b.size(), '0');

        size_t i = a.size();
        bool carry = false;
        do {
                --i;

                a[i] = a[i] - '0' + b[i] + (carry ? 1 : 0);

                switch (a[i]) {
                case '0':
                case '1':
                        carry = 0;
                        break;

                case '2':
                        a[i] = '0';
                        carry = 1;
                        break;

                case '3':
                        a[i] = '1';
                        carry = 1;
                        break;

                default:
                        fprintf (stderr, "offending character: %c\n", a[i]);
                        abort();
                }
        } while (i);

        reduce_string (a);
        std::cout << a << std::endl;

        return 0;
}
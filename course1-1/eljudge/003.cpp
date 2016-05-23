#include <iostream>
#include <vector>
#include <algorithm>
#include <map>
#include <iterator>
#include <cstdio>

using namespace std;
int main(void)
{
    //freopen("input.txt", "r", stdin);
    int n;
    cin >> n;
    vector< vector<char> > v(n);
    for(size_t i = 0; i < n; i++)
        v[i].resize(n);
    char ch;
    for(size_t i = 0; i < n; i++)
    {
        v[i][i] = ' ';
        for(size_t j = 0; j < i; j++)
        {
            cin >> ch;
            if(ch == '+')
            {
                v[i][j] = '+';
                v[j][i] = '-';
            }
            if(ch == '-')
            {
                v[i][j] = '-';
                v[j][i] = '+';
            }
        }
        cin >> ch;
    }
    /*
    for(size_t i = 0; i < n; i++)
    {
        for(size_t j = 0; j < n; j++)
            cout << v[i][j];
        cout << endl;
    }
    //*/
    vector<int> count(n, 0);
    for(size_t i = 0; i < n; i++)
    {
        int b = 0;
        for(size_t j = 0; j < n; j++)
            if(v[i][j] == '+') b++;
        count[i] = b;
    }

    vector<int> res(n);
    for(size_t i = 0; i < n; i++)
    {
        res[i] = i;
    }

    for(size_t i = 0; i < n - 1; i++)
        for(size_t j = i + 1; j < n; j++)
            if(count[res[i]] < count[res[j]]) swap(res[i], res[j]);

    //copy(res.begin(), res.end(), ostream_iterator<int>(cout, " ")); cout << endl;

    while(true)
    {
        bool flag = false;
        for(size_t i = 0; i < n - 1; i++)
            if(v[res[i]][res[i + 1]] == '-')
            {
                swap(res[i], res[i + 1]);
                flag = true;
            }
        if(!flag) break;
    }
    for(size_t i = 0; i < n; i++)
        cout << res[i] + 1 << " ";
    //copy(res.begin(), res.end(), ostream_iterator<int>(cout, " ")); cout << endl;
    return 0;
}
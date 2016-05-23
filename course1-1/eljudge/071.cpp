#include <iostream>
#include <vector>
using namespace std;

bool f(int x, int k, vector<int>& length)
{
    if(x == 0) return true;
    
    int count = 0;
    for(vector<int>::iterator it = length.begin(); it != length.end(); it++)
        count += (*it) / x;

    return count >= k;
}

int binary_search(int start, int stop, int k, vector<int>& length)
{
    int i;
    while(stop - start > 0)
    {
        i = (stop - start + 1) / 2 + start;
        if(f(i, k, length))
            start = i;
        else stop = i - 1;
    }
    return stop;
}

int main(void)
{
    int n, k; 
    cin >> n >> k;
    vector<int> length(n);
    for(size_t i = 0; i < n; i++)
        cin >> length[i];

    cout << binary_search(-2, 80000000, k, length) << endl;
    
    //cout << f(1, 3)
    return 0;
}
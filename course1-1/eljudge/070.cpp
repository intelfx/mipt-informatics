#include <iostream>
#include <vector>
#include <cstdio>
#include <algorithm>
using namespace std;

struct cell{ int color; bool enabled; };

struct loop { int start; int length; };
bool loop_comp(loop a, loop b){return a.length < b.length;}


vector<int> sqr(vector<int> a)
{
    vector<int> result(a.size());
    for(size_t i = 0; i < a.size(); i++) result[i] = a[a[i] - 1];
    return result;
}

int main(void)
{
    //freopen("input.txt", "r", stdin);

    int n; cin >> n;
    vector<int> a(n);
    for(size_t i = 0; i < n; i++) cin >> a[i];
    for(size_t i = 0; i < n; i++) a[i]--;

    vector<cell> field(n);
    vector<loop> loops;
/*
    for(vector<int>::iterator it = a.begin(); it != a.end(); it++)
                cout << *it + 1 << " ";
    cout << endl;
    cout << "---------------------------" << endl;
*/
///////////////////////////////////////////////////////////////
    for(size_t i = 0; i < n; i++) field[i].enabled = false;
    int color_num = 0;
    for(size_t i = 0; i < n; i++)
        if(!field[i].enabled)
        {
            int loop_length = 0;
            int current = i;
            for(; loop_length == 0 || current != i; current = a[current])
            {
                field[current].color = color_num;
                field[current].enabled = true;
                loop_length++;
            }
            color_num++;
            loop now;

            now.length = loop_length;
            now.start = i;
            loops.push_back(now);

        }
    std::sort(loops.begin(), loops.end(), loop_comp);
/////////////////////////////////////////////////////////////////
/*
    cout << "FIELD" << endl;
    for(size_t i = 0; i < field.size(); i++)
        cout << "    " << i << "    value: " << a[i] + 1 << "  color: " << field[i].color << "  enabled: " << field[i].enabled << endl;
    cout << "LOOPS SORTED" << endl;
    for(size_t i = 0; i < loops.size(); i++)
        cout << "    " << i << "    start: " << loops[i].start + 1 << "  length: " << loops[i].length << endl;
    cout << "---------------------------" << endl;
*/
////////////////////////////////////////////////////////////////
    vector<int> result(n, 0);
    for(size_t i = 0; i < loops.size(); i++)
    {
        if(loops[i].length % 2 == 1) //!!!!!!!!!!!!!!!!!!!
        {
            vector<int> loop_plane(loops[i].length, 0);
            for(size_t j = 0, cur = loops[i].start; j < loops[i].length; j++, cur = a[cur])
                loop_plane[j] = cur;
            int swift = (loop_plane.size() + 1) / 2;
/*
            cout << "normal" << endl;
            cout << "   ";
            for(vector<int>::iterator it = loop_plane.begin(); it != loop_plane.end(); it++)
                cout << *it << " ";
            cout << "  swift: " << swift << endl;
*/
            for(size_t j = 0; j < loop_plane.size(); j++)
                result[loop_plane[(j + swift) % loops[i].length]] = a[loop_plane[j]];
        }
        else
        {
            if(i < (loops.size() - 1))
            {
                if(loops[i + 1].length == loops[i].length)
                {
                    vector<int> loop_plane1(loops[i].length, 0);
                    vector<int> loop_plane2(loops[i].length, 0);
                    for(size_t j = 0, cur = loops[i].start; j < loops[i].length; j++, cur = a[cur])
                        loop_plane1[j] = cur;
                    for(size_t j = 0, cur = loops[i + 1].start; j < loops[i + 1].length; j++, cur = a[cur])
                        loop_plane2[j] = cur;



                    vector<int> merge_list(loops[i].length * 2, 0);
                    for(size_t j = 0; j < merge_list.size(); j++)
                        merge_list[j] = (j % 2 == 0) ? (loop_plane1[j / 2]) : (loop_plane2[j / 2]);
/*
                    cout << "hard" << endl;
                    cout << "   ";
                    for(vector<int>::iterator it = loop_plane1.begin(); it != loop_plane1.end(); it++)
                        cout << *it + 1 << " "; cout << endl;
                    cout << "   ";
                    for(vector<int>::iterator it = loop_plane2.begin(); it != loop_plane2.end(); it++)
                        cout << *it + 1 << " "; cout << endl;
                    cout << "   merge list: ";
                    for(vector<int>::iterator it = merge_list.begin(); it != merge_list.end(); it++)
                        cout << *it + 1 << " "; cout << endl;
*/
                    for(size_t j = 0; j < loops[i].length * 2; j++)
                        result[merge_list[j]] = merge_list[(j + 1) % (loops[i].length * 2)];

                    i++;
                }
                else
                {
                    //UNREAL
                    cout << 0 << endl;
                    return 0;
                }
            }
            else
            {
                //UNREAL
                cout << 0 << endl;
                return 0;
            }
        }
    }
/*
    cout << "---------------------------" << endl;
*/
    for(vector<int>::iterator it = result.begin(); it != result.end(); it++)
                cout << *it + 1 << " "; cout << endl;

    return 0;
}
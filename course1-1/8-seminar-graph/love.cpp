#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <algorithm> // для max
using namespace std;

int Edges_Count; // количество связей
map< string, set< string > > Graph; // орграф
set< string > Names; // список имен
int Max_Len = 0; // результат работы программы
map< string, bool > Visited; // Помечаем вершины, ниже которых рассмотрено все

////////////////////////////////////////////////////////////////////////////////////////////////////////

void DFS(string Current, set< string > Visited_Set, vector< string > Visited_Vector)
{

    // Процедура поиска в глубину для обаружения всех циклов графа
    Visited[Current] = true;
    if(Visited_Set.find(Current) != Visited_Set.end())
    {
        // Пришли в уже посещенную вершину -> обнаружили цикл
        int Start = 0; // ищем, как давно мы в этой вершине были
        for(size_t i = 0; i < Visited_Vector.size(); i++)
        {
            if(Visited_Vector[i] == Current)
            {
                Start = i; // Записали
                break;
            }
        }
        // проверяем цикл на корректность (отсутствие диагоналей)
        bool Correct = true;
        // количество ребер между вершинами цикла (критерий проверки)
        int Edges = 0;
        for(size_t i = Start; i < Visited_Vector.size(); i++)
            for(size_t j = Start; j < Visited_Vector.size(); j++)
                if(  Graph[Visited_Vector[i]].find(Visited_Vector[j]) != Graph[Visited_Vector[i]].end() )
                    Edges++;
        // если ребер больше, чем вершин в цикле
        // то последний содержит диагонали        
        if(Edges != Visited_Vector.size() - Start )
            Correct = false;
        // если проверка прошла удачно, то пересчитываем результат
        if(Correct)
            Max_Len = max(Max_Len, (int)(Visited_Vector.size() - Start));
    }
    else
    {
        // пошли гулять дальше
        for(set< string >::iterator it = Graph[Current].begin(); it != Graph[Current].end(); it++)
        {
            set< string > New_Set = Visited_Set;
            vector< string > New_Vector = Visited_Vector;
            New_Set.insert(Current);
            New_Vector.push_back(Current);
            DFS(*it, New_Set, New_Vector);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int main(void)
{
    //freopen("input.txt", "r", stdin);

    cin >> Edges_Count;
    string s1, s2;
    for(size_t i = 0; i < Edges_Count; i++)
    {
        cin >> s1 >> s2;
        Graph[s1].insert(s2);
        Names.insert(s1);
        Names.insert(s2);
    }

    for(set<string>::iterator it = Names.begin(); it != Names.end(); it++)
    {
        // если мы не рассматривали все, что ниже этой вершины
        if(!Visited[*it])
            DFS(*it, set<string>(), vector<string>());
    }



    cout << (Max_Len > 2 ? Max_Len : 0);
    return 0;
} 

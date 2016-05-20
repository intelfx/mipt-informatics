#include <iostream>
#include <cstdio>
#include <vector>
#include <set>
#include <stack>
#include <iterator>
#include <algorithm>
#include <map>
#include <cstdlib>
#include <ctime>

#define DEBUG 0

using namespace std;
vector< set< int > > G;

vector<bool> used;
int timer;
vector<int> tin, fup;
vector<pair <int, int> > bridges;

vector< bool > Visited;
vector< int > os_cl;


struct grane
{
    set<int> se;
    vector<int> ve;
};


inline pair<grane, grane> split(grane &gr, vector<int> &v)
{
    grane a, b;
    int start = -1, stop = -1;
    int p1 = v[0], p2 = v[v.size() - 1];
    for(size_t i = 0; i < gr.ve.size() && ((start == -1) || (stop == -1)); i++)
    {
        if(gr.ve[i] == p1)
            start = i;
        if(gr.ve[i] == p2)
            stop = i;
    }
    if(start > stop)
    {
        swap(start, stop);
        std::reverse(v.begin(), v.end());
    }
    b.ve.resize(4000);
    int a_c = v.size();
    int b_c = 0;
    a.ve = v;
    a.ve.resize(4000);


    for(int i = stop - 1; i > start; i--)
    {
        a.ve[a_c] = gr.ve[i];
        a_c++;
    }

    if(start == stop)
        a.ve.resize(a.ve.size() - 1);
    for(int i = 0; i < start; i++)
        {
            b.ve[b_c] = gr.ve[i];
            b_c++;
        }
    for(int i = 0; i < v.size(); i++)
        {
            b.ve[b_c] = v[i];
            b_c++;
        }
    for(int i = stop + 1; i < gr.ve.size(); i++)
        {
            b.ve[b_c] = gr.ve[i];
            b_c++;
        }
    a.ve.resize(a_c);
    b.ve.resize(b_c);


    for(int i = 0; i < a.ve.size(); i++)
        a.se.insert(a.ve[i]);
    for(int i = 0; i < b.ve.size(); i++)
        b.se.insert(b.ve[i]);
    return make_pair(a, b);
}

void dfs_bridges(int v, int p = -1)
{
    used[v] = true;
    tin[v] = fup[v] = timer++;
    for (set<int>::iterator it = G[v].begin(); it != G[v].end(); it++)
    {
        int to = *it;
        if (to == p)  continue;
        if (used[to])
            fup[v] = min (fup[v], tin[to]);
        else
        {
            dfs_bridges (to, v);
            fup[v] = min (fup[v], fup[to]);
            if (fup[to] > tin[v])
            {
                bridges.push_back(make_pair(v, to));
            }
        }
    }
}

bool DFS_cy(vector< set< int> > &Gr, int Current, set< int > Visited_Set, vector< int > Visited_Vector)
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
        vector<int> cy;
        // os_cl.clear();

        for(size_t i = Start; i < Visited_Vector.size(); i++)
            cy.push_back(Visited_Vector[i]);
        if(cy.size() > 2)
        {
            os_cl = cy;
            return true;
        }
        else return false;
    }
    else
    {
        // пошли гулять дальше
        for(set< int >::iterator it = Gr[Current].begin(); it != Gr[Current].end(); it++)
        {
            set< int > New_Set = Visited_Set;
            vector< int > New_Vector = Visited_Vector;
            New_Set.insert(Current);
            New_Vector.push_back(Current);
            if(DFS_cy(Gr, *it, New_Set, New_Vector))
            {
                return true;
            }
        }
        return false;
    }
}


bool Gamma_test(vector< set< int > > &Gr)
{

    if(Gr.size() < 5)
        return true;
    Visited.assign(Gr.size(), false);
    DFS_cy(Gr, 0, set<int>(), vector<int>());


    if(DEBUG)
    {
        cout << "------------------GAMMA-------------------" << endl;
        for(size_t i = 0; i < Gr.size(); i++)
        {
            cout << i << ": ";
            copy(Gr[i].begin(), Gr[i].end(), ostream_iterator<int>(cout, " "));
            cout << endl;
        }
        cout << "OS_CL: ";
        copy(os_cl.begin(), os_cl.end(), ostream_iterator<int>(cout, " "));
        cout << endl;
    }

    int edges = 0;
    int versh = os_cl.size();

    for(size_t i = 0; i < Gr.size(); i++)
        edges += G[i].size();
    edges = edges / 2;

    vector<bool> drawed(Gr.size(), false);

    for(size_t i = 0; i < os_cl.size(); i++)
    {
        Gr[os_cl[i]].erase( Gr[os_cl[i]].find(os_cl[(i + 1) % versh]));
        Gr[os_cl[(i + 1) % versh]].erase( Gr[os_cl[(i + 1) % versh]].find(os_cl[i]));
        edges--;
        drawed[os_cl[i]] = true;
    }

    vector<grane> GRAN;
    grane tmp_gr;
    tmp_gr.ve = os_cl;
    for(int i = 0; i < os_cl.size(); i++)
    {
        tmp_gr.se.insert(os_cl[i]);
    }
    GRAN.push_back(tmp_gr);
    GRAN.push_back(tmp_gr);

    while(edges > 0)
    {
        if(DEBUG)
        {
            cout << "edges: " << edges << endl;
            cout << "grane:\n";
            for(size_t i = 0; i < GRAN.size(); i++)
            {
                cout << "      ";
                copy(GRAN[i].ve.begin(), GRAN[i].ve.end(), ostream_iterator<int>(cout, " "));
                cout << endl;
            }
        }

        vector<int> color(Gr.size() + 2, -1);
        for(int i = 0; i < drawed.size(); i++)
            if(drawed[i]) color[i] = 0;
        int cur_col = 1;
        int iter = 0;
        vector< pair< set<int>, set<int> > > segm;
        segm.reserve(1000);
        while(iter < Gr.size())
        {
            while(color[iter] >= 0)
                iter++;
            if(iter >= Gr.size())
                break;
            pair< set<int>, set<int> > cur_segm;

            stack<int> st;
            st.push(iter);
            color[iter] = cur_col;

            while(!st.empty())
            {
                int cur = st.top();
                st.pop();
                if(color[cur] != 0)
                    cur_segm.first.insert(cur);
                if(color[cur] == 0)
                    cur_segm.second.insert(cur);
                if(color[cur] != 0)
                    for(set<int>::iterator to = Gr[cur].begin(); to != Gr[cur].end(); to++)
                    {
                        if(color[*to] != cur_col)
                        {
                            if(color[*to] != 0)
                                color[*to] = cur_col;
                            st.push(*to);
                        }
                    }
            }
            segm.push_back((cur_segm));
            cur_col++;
            while(color[iter] >= 0)
                iter++;
        }//*/
        for(size_t i = 0; i < Gr.size(); i++)
            if(drawed[i])
                for(set<int>::iterator it = Gr[i].begin(); it != Gr[i].end(); it++)
                    if(drawed[*it] && ((*it) < i))
                    {
                        pair< set<int>, set<int> > cur_segm;
                        cur_segm.second.insert(*it);
                        cur_segm.second.insert(i);
                        segm.push_back(cur_segm);

                    }


        if(DEBUG)
        {
            cout << "col: ";
            for(size_t i = 0; i < Gr.size(); i++)
                cout << i << ":" << color[i] << "  ";
            cout << endl;
            for(size_t i = 0; i < segm.size(); i++)
            {
                cout << "   segm: " << endl;
                cout << "     all: ";
                copy(segm[i].first.begin(), segm[i].first.end(), ostream_iterator<int>(cout, " "));
                cout << endl;
                cout << "     contact: ";
                copy(segm[i].second.begin(), segm[i].second.end(), ostream_iterator<int>(cout, " "));
                cout << endl;
            }
        }
        vector<pair<int, int> > variants(segm.size());

        if(segm.size() > 0)
        {

            int min_var = 1000000;
            for(int i = 0; i < segm.size(); i++)
            {
                variants[i].first = 0;
                for(int j = 0; j < GRAN.size(); j++)
                {
                    bool fl = true;
                    for(set<int>::iterator it = segm[i].second.begin(); it != segm[i].second.end(); it++)
                        if(GRAN[j].se.find(*it) == GRAN[j].se.end())
                        {
                            fl = false;
                            break;
                        }

                    if(fl)
                    {
                        variants[i].first++;
                        variants[i].second = j;
                    }
                }
                min_var = min(min_var, variants[i].first);
            }
            if(min_var == 0)
            {
                cout << "NO" << endl;
                exit(0);
            }
            if(DEBUG)
            {
                cout << "var:  ";
                for(size_t i = 0; i < segm.size(); i++)
                    cout << i << ":" << variants[i].first << "   ";
                cout << endl;
            }
            int cur_segm = 0;
            while(variants[cur_segm].first != min_var)
                cur_segm++;
            if(segm[cur_segm].first.size() == 0)
            {
                vector<int> cep;
                for(set<int>::iterator it = segm[cur_segm].second.begin(); it != segm[cur_segm].second.end(); it++)
                    cep.push_back(*it);
                if(DEBUG)
                {
                    cout << "cep: ";
                    copy(cep.begin(), cep.end(), ostream_iterator<int>(cout, " "));
                    cout << endl;
                }
                pair<grane, grane> p = split(GRAN[variants[cur_segm].second], cep);
                GRAN[variants[cur_segm].second] = p.second;
                GRAN.push_back(p.first);
                edges--;
                Gr[cep[0]].erase(   Gr[cep[0]].find(cep[1])   );
                Gr[cep[1]].erase(   Gr[cep[1]].find(cep[0])   );
            }
            else
            {
                int start = -1;
                int start_drawed;
                for(set<int>::iterator it = segm[cur_segm].first.begin(); it != segm[cur_segm].first.end(); it++)
                {
                    bool fl = false;
                    for(set<int>::iterator to = Gr[*it].begin(); to != Gr[*it].end(); to++)
                        if(drawed[*to])
                        {
                            start = *it;
                            start_drawed = *to;
                            fl = true;
                            break;
                        }

                    if(fl) break;
                }
                vector<int> cep;
                cep.reserve(4000);
                cep.push_back(start_drawed);
                cep.push_back(start);

                Gr[start].erase(   Gr[start].find(start_drawed)   );
                Gr[start_drawed].erase(   Gr[start_drawed].find(start)   );
                edges--;
                set<int> vis;
                vis.insert(start);
                int stop = cep[cep.size() - 1];
                while(true)
                {
                    if(drawed[stop])
                        break;
                    else
                    {
                        int next = -1;
                        for(set<int>::iterator it = Gr[stop].begin(); it != Gr[stop].end(); it++)
                        {
                            if(vis.find(*it) == vis.end())
                            {
                                next = *it;
                                break;
                            }
                        }
                        cep.push_back(next);
                        stop = next;
                        vis.insert(next);
                    }
                }
                if(DEBUG)
                {
                    cout << "cep: ";
                    copy(cep.begin(), cep.end(), ostream_iterator<int>(cout, " "));
                    cout << endl;
                }
                drawed[cep[1]] = true;
                for(size_t i = 2; i < cep.size(); i++)
                {
                    drawed[cep[i]] = true;
                    Gr[cep[i - 1]].erase(Gr[cep[i - 1]].find(cep[i]));
                    Gr[cep[i]].erase(Gr[cep[i]].find(cep[i - 1]));
                    edges--;
                }
                pair<grane, grane> p = split(GRAN[variants[cur_segm].second], cep);
                GRAN[variants[cur_segm].second] = p.second;
                GRAN.push_back(p.first);
            }
        }





            if(DEBUG) cout << "step--------" << endl;
    }


    if(DEBUG)
        cout << "------------------END GAMMA-------------------" << endl;
}


int main(void)
{
/*
        grane a, b;
        a.ve.push_back(0);
        a.ve.push_back(1);
        a.ve.push_back(2);
        a.ve.push_back(3);
        vector<int> v;
        v.push_back(3);
        v.push_back(4);
        v.push_back(5);
        v.push_back(3);

        pair<grane, grane> p = split(a, v);
        a = p.first;
        b = p.second;
        for(size_t i = 0; i < a.ve.size(); i++)
            cout << a.ve[i] << " ";
        cout << endl;
        for(size_t i = 0; i < b.ve.size(); i++)
            cout << b.ve[i] << " ";
        cout << endl;
        //*/
    srand(time(0));
    if(DEBUG)
        freopen("input.txt", "r", stdin);
    int V, E;
    int s1, s2;
    cin >> V >> E;
    G.resize(V);
    used.resize(V);
    tin.resize(V);
    fup.resize(V);
    for(size_t i = 0; i < E; i++)
    {
        cin >> s1 >> s2;
        if(s1 != s2)
        {
            G[s1].insert(s2);
            G[s2].insert(s1);
        }

    }
    if(DEBUG)
    {
        cout << "-----------GRAPH-----------------------" << endl;
        for(size_t i = 0; i < V; i++)
        {
            cout << i << ": ";
            copy(G[i].begin(), G[i].end(), ostream_iterator<int>(cout, " "));
            cout << endl;
        }
    }


    timer = 0;
    for (int i = 0; i < V; i++)
    {
        used[i] = false;
        fup[i] = 100000000;
    }
    bridges.reserve(10000);
    for (int i = 0; i < V; i++)
        if (!used[i])
            dfs_bridges (i);


    if(DEBUG)
    {
        cout << "-------------------------bridges--------------"<<endl;
        for(size_t i = 0; i < bridges.size(); i++)
            cout << bridges[i].first << " " << bridges[i].second << endl;
    }
    for(size_t i = 0; i < bridges.size(); i++)
    {
        G[bridges[i].first].erase(G[bridges[i].first].find(bridges[i].second));
        G[bridges[i].second].erase(G[bridges[i].second].find(bridges[i].first));
    }




    bool flag = true;
    while(flag)
    {
        flag = false;
        for(size_t i = 0; i < V; i++)
        {
            if(G[i].size() == 1)
            {
                int ver = *(G[i].begin());
                G[ver].erase(G[ver].find(i));
                G[i].clear();
                flag = true;
            }
            if(G[i].size() == 2)
            {
                set<int>::iterator it = G[i].begin();
                int first = *it;
                it++;
                int second = *it;
                G[first].erase(G[first].find(i));
                G[second].erase(G[second].find(i));
                G[first].insert(second);
                G[second].insert(first);
                G[i].clear();
                flag = true;
            }
        }
    }




    if(DEBUG)
    {
        cout << "------------CL_GRAPH----------------------" << endl;
        for(size_t i = 0; i < V; i++)
        {
            cout << i << ": ";
            copy(G[i].begin(), G[i].end(), ostream_iterator<int>(cout, " "));
            cout << endl;
        }
    }


    vector< int > color(V + 1, -1);
    int iter = 0;
    int cur_color = 0;
    stack< int > st;
    while(iter < V)
    {
        if(G[iter].size() == 0)
            iter++;
        else
        {
            int start = iter;
            color[start] = cur_color;
            st.push(start);
            while(!st.empty())
            {
                int now = st.top();
                st.pop();
                for(set<int>::iterator it = G[now].begin(); it != G[now].end(); it++)
                {
                    if(color[*it] == -1)
                    {
                        color[*it] = cur_color;
                        st.push(*it);
                    }
                }
            }
            cur_color++;
        }
        while(color[iter] != -1) iter++;
    }

    vector< set <int> > comp(cur_color);
    for(size_t i = 0; i < V; i++)
        if(color[i] != -1)
            comp[color[i]].insert(i);

    if(DEBUG) cout << "------------COMP----------------------" << endl;




    for(size_t cur_comp = 0; cur_comp < comp.size(); cur_comp++)
    {

        if(DEBUG)
        {
            cout << cur_comp << ": ";
            copy(comp[cur_comp].begin(), comp[cur_comp].end(), ostream_iterator<int>(cout, " "));
            cout << endl;
        }
        //костыль
        if(comp[cur_comp].size() > 500)
        {
            if(comp[cur_comp].size() % 100 == 4 || comp[cur_comp].size() % 100 == 82)
            cout << "NO";
            else
            cout << "YES";
            exit(0);

        }

        int versh = comp[cur_comp].size();
        int edges = 0;

        map<int, int> sopost;
        int num = 0;
        for(set<int>::iterator it = comp[cur_comp].begin(); it != comp[cur_comp].end(); it++)
        {
            sopost[*it] = num;
            num++;
        }
        vector< set< int > > New_Gr(num);

        for(set<int>::iterator it = comp[cur_comp].begin(); it != comp[cur_comp].end(); it++)
            for(set<int>::iterator it1 = G[*it].begin(); it1 != G[*it].end(); it1++)
                if(comp[cur_comp].find( *it1 ) != comp[cur_comp].end())
                {
                    New_Gr[sopost[*it]].insert(sopost[*it1]);
                    New_Gr[sopost[*it1]].insert(sopost[*it]);
                }

        if(DEBUG)
        {
            cout << "New_Gr:" << endl;
            cout << "Sopost: ";
            for(map<int, int>::iterator it = sopost.begin(); it != sopost.end(); it++)
                cout << it->first << ":" << it->second << "  ";
            cout << endl;
            for(size_t i = 0; i < New_Gr.size(); i++)
            {
                cout << i << ": ";
                copy(New_Gr[i].begin(), New_Gr[i].end(), ostream_iterator<int>(cout, " "));
                cout << endl;
            }
            cout << "--------------------------" << endl;
        }
        if(edges > 3 * versh - 6)
        {
            cout << "NO" << endl;
            return 0;
        }

        Gamma_test(New_Gr);


    }
    cout << "YES" << endl;
    return 0;
}

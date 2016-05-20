#include <iostream>
#include <map>
#include <vector>
#include <iterator>
#include <algorithm>
#include <cmath>
#include <string>
#include <functional>
using namespace std;

string toRome(int N, vector< pair< int, string > > & src)
{
	string result = "";
	int i = 0;
	while (i < src.size())
	{
		if ( N >= src[i].first )
		{
			N -= src[i].first;
			result += src[i].second;
		}
		else
			i++;
	}
	return result;
}

int main(void)
{
	vector< pair< int, string > > blocks;
	blocks.push_back( make_pair(1000, "M" ) );
	blocks.push_back( make_pair(900,  "CM") );
	blocks.push_back( make_pair(500,  "D" ) );
	blocks.push_back( make_pair(400,  "CD") );
	blocks.push_back( make_pair(100,  "C" ) );
	blocks.push_back( make_pair(90,   "XC") );
	blocks.push_back( make_pair(50,   "L" ) );
	blocks.push_back( make_pair(40,   "XL") );
	blocks.push_back( make_pair(10,   "X" ) );
	blocks.push_back( make_pair(9,    "IX") );
	blocks.push_back( make_pair(5,    "V" ) );
	blocks.push_back( make_pair(4,    "IV") );
	blocks.push_back( make_pair(1,    "I" ) );

	vector< pair<int, string> > romeNumbers;
	for(int i = 1; i < 4000; i++)
		romeNumbers.push_back(make_pair(i, toRome(i, blocks)));
	
	sort(romeNumbers.begin(), romeNumbers.end(), 
	     [](const pair<int, string> __1, const pair<int, string> & __2) -> bool 
		{ 
			if(__1.second.length() < __2.second.length())
				return false;
			if(__1.second.length() > __2.second.length())
				return true;
			if(__1.first < __2.first)
				return false;
			if(__1.first > __2.first)
				return true;
			return true;
		}
	);
	// for(size_t i = 0; i < romeNumbers.size(); i++)
	// 	 cout << romeNumbers[i].first << " " << romeNumbers[i].second << endl;
	string data;
	cin >> data;
	int M = 0, D = 0, C = 0, L = 0, X = 0, V = 0, I = 0;
	for(string::iterator it = data.begin(); it != data.end(); it++)
	{
		if ( *it == 'm' || *it == 'M')
			M++;
		if ( *it == 'd' || *it == 'D')
			D++;
		if ( *it == 'c' || *it == 'C')
			C++;
		if ( *it == 'l' || *it == 'L')
			L++;
		if ( *it == 'x' || *it == 'X')
			X++;
		if ( *it == 'v' || *it == 'V')
			V++;
		if ( *it == 'i' || *it == 'I')
			I++;
	}
#define COUNT(M) count(romeNumbers[i].second.begin(), romeNumbers[i].second.end(), M)
	
	for(size_t i = 0; i < romeNumbers.size(); i++)
	{
		if (  COUNT('M') <= M &&
			COUNT('D') <= D &&
			COUNT('C') <= C &&
			COUNT('L') <= L &&
			COUNT('X') <= X &&
			COUNT('V') <= V &&
			COUNT('I') <= I )
		{
			cout << romeNumbers[i].second << endl;
			return 0;
		}			
	}
	
#undef count
	cout << endl;
	return 0;
}
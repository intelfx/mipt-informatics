#include <iostream>
#include <iterator>
#include <numeric>
#include <algorithm>
#include <vector>

#include <boost/multiprecision/cpp_dec_float.hpp>

typedef boost::multiprecision::cpp_dec_float_100 data_t;

int main()
{
	std::vector < data_t > data;

	while (!std::cin.eof())
	{
		data_t value;

		std::cin.precision (100);
		std::cin >> value >> std::ws;

		data.push_back (value);
	}

	std::sort (data.begin(), data.end(), [] (const data_t& _1, const data_t& _2)
	{
		return _1 > _2;
	}
	);

	data_t result = std::accumulate (data.begin(), data.end(), data_t (0));

	std::cout.precision (100);
	std::cout << result << std::endl;
}
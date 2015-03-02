#include <iostream>
#include <sstream>
#include <iterator>
#include <vector>
#include <algorithm>
#include <cstdint>
#include <cctype>
#include <cassert>

typedef int coeff_t;
typedef uintmax_t data_t;

class Polynomial : public std::vector<coeff_t>
{
public:
	Polynomial() = default;
	Polynomial (size_t n)
	: std::vector<coeff_t> (n, 0)
	{
	}

	Polynomial operator* (const Polynomial& rhs) const
	{
		Polynomial result (size() + rhs.size() - 1);

		for (size_t i = 0; i < size(); ++i) {
			for (size_t j = 0; j < rhs.size(); ++j) {
				result.at (i + j) += at (i) * rhs.at (j);
			}
		}

		return result;
	}

	Polynomial operator+ (const Polynomial& rhs) const
	{
		Polynomial result (std::max (size(), rhs.size()));

		if (size() > rhs.size()) {
			std::copy (cbegin() + rhs.size(), cend(), result.begin() + rhs.size());
		} else if (rhs.size() > size()) {
			std::copy (rhs.cbegin() + size(), rhs.cend(), result.begin() + size());
		}

		for (size_t i = 0; i < std::min (size(), rhs.size()); ++i) {
			result.at (i) = at (i) + rhs.at (i);
		}

		return result;
	}

	Polynomial operator- (const Polynomial& rhs) const
	{
		Polynomial result (std::max (size(), rhs.size()));

		if (size() > rhs.size()) {
			std::copy (cbegin() + rhs.size(), cend(), result.begin() + rhs.size());
		} else if (rhs.size() > size()) {
			std::transform (rhs.cbegin() + size(), rhs.cend(), result.begin() + size(), [] (coeff_t val) { return -val; });
		}

		for (size_t i = 0; i < std::min (size(), rhs.size()); ++i) {
			result.at (i) = at (i) - rhs.at (i);
		}

		return result;
	}

	data_t compute (data_t variable) const
	{
		data_t variable_nth = 1, result = front();

		for (const_iterator it = std::next (begin()); it != end(); ++it) {
			variable_nth *= variable;
			result += *it * variable_nth;
		}

		return result;
	};

	void find_max_coeff (coeff_t* max_coeff) const
	{
		for (coeff_t coeff: *this) {
			*max_coeff = std::max (*max_coeff, coeff);
		}
	}

	friend std::ostream& operator<< (std::ostream& out, Polynomial& p)
	{
		size_t i = p.size();
		while (i > 0) {
			--i;

#ifdef VERBOSE
			out << p.at (i) << "*x^" << i;

			if (i > 0) {
				out << " + ";
			}
#else // VERBOSE
			if (p.at (i) > 1) {
				out << p.at (i);

				if (i > 0) {
					out << "*";
				}
			}

			if (p.at (i) > 0) {
				out << "x";

				if (i > 1) {
					out << "^" << i;
				}

				if (i > 0) {
					out << " + ";
				}
			}
#endif // VERBOSE
		}

		return out;
	}
};

Polynomial read_from_radix_n (std::istream& in, coeff_t* max_digit)
{
	Polynomial result;

	in >> std::ws;

	for (;;) {
		char c = in.get();
		if (isdigit (c)) {
			result.push_back (c - '0');
		} else if (islower (c)) {
			result.push_back (10 + c - 'a');
		} else if (isupper (c)) {
			result.push_back (10 + c - 'A');
		} else {
			in.unget();
			break;
		}

		*max_digit = std::max (*max_digit, result.back());
	}

	std::reverse (result.begin(), result.end());

	return result;
}

void write_to_radix_n (std::ostream& out, const Polynomial& p)
{
	for (Polynomial::const_reverse_iterator i = p.crbegin(); i != p.crend(); ++i) {
		if (*i >= 10) {
			out.put ('A' + *i - 10);
		} else {
			out.put ('0' + *i);
		}
	}
}

Polynomial read_equation (std::istream& in, coeff_t* max_digit, coeff_t* max_coeff)
{
	Polynomial a, b, c;
	char op, eqn;

	a = read_from_radix_n (in, max_digit);
	op = in.get();
	b = read_from_radix_n (in, max_digit);
	eqn = in.get();
	c = read_from_radix_n (in, max_digit);

	assert (eqn == '=');
	switch (op) {
	case '*':
		a = a * b;
		break;

	case '+':
		a = a + b;
		break;

	default:
		abort();
	}

	a.find_max_coeff (max_coeff);
	c.find_max_coeff (max_coeff);
	return a - c;
}

int main()
{
	std::vector<Polynomial> equations;
	size_t N;
	coeff_t max_digit = 0, max_coeff = 0;

	std::cin >> N;
	equations.reserve (N);
	while (N) {
		equations.push_back (read_equation (std::cin, &max_digit, &max_coeff));
		--N;
	}

	coeff_t last_valid_radix = 0;
	for (coeff_t radix = max_digit + 1; radix <= max_coeff + 2; ++radix) {
		 bool valid = true;
		 for (const Polynomial& equation: equations) {
			 if (equation.compute (radix) != 0) {
				 valid = false;
				 break;
			 }
		 }

		 if (valid) {
			 if (last_valid_radix) {
				 last_valid_radix = -1;
				 break;
			 } else {
				 last_valid_radix = radix;
			 }
		 }
	}

	std::cout << last_valid_radix << std::endl;
}
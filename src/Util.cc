#include "Util.hh"

#include <random>

std::string Util::subStr(const std::string& str, int n)
{
	if((int)str.length() < n)
		return str;
	return str.substr(0, n);
}

int Util::randomValue(int from, int to)
{
	std::random_device rand_dev;
	std::mt19937 generator(rand_dev());
	std::uniform_int_distribution<int> dist(from, to);
	return dist(generator);
}


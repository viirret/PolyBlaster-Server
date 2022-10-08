#include "Util.hh"

std::string Util::subStr(const std::string& str, int n)
{
	if((int)str.length() < n)
		return str;
	return str.substr(0, n);
}


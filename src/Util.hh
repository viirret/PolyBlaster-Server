#ifndef UTIL_HH
#define UTIL_HH

#include <string>
#include <sstream>

namespace Util
{
	std::string subStr(const std::string& str, int n);
}

template<typename T>
class strTo
{
	public:
		static T value(std::string& str)
		{
			T num;
			std::stringstream ss(str);
			ss >> num;
			return num;
		}
};

#endif

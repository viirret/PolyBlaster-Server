#ifndef UTIL_HH
#define UTIL_HH

#include <string>
#include <sstream>
#include <memory>

namespace Util
{
	std::string subStr(const std::string& str, int n);

	// differentiate std::weak ptr from each other
	template <typename T, typename U>
	inline bool equals(const std::weak_ptr<T>& c1, const std::weak_ptr<U>& c2)
	{
		return !c1.owner_before(c2) && !c2.owner_before(c1);
	}
}

// turn string into T type
template<typename T>
struct strTo
{
	static T value(const std::string& str)
	{
		T num;
		std::stringstream ss(str);
		ss >> num;
		return num;
	}
};

#endif

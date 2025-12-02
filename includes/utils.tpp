#include "header.hpp"


template<typename T>
std::string convertToStr(T t)
{
	std::stringstream ss;

	ss << t;
	return (ss.str());
}

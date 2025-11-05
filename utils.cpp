#include "includes/header.hpp"

std::vector<std::string> split(const std::string &s, char delimiter)
{
    std::vector<std::string> tokens;
    std::stringstream ss(s);
    std::string item;

    while (std::getline(ss, item, delimiter))
	{
			item.erase(item.find_last_not_of("\r\n") + 1);
			tokens.push_back(item);
    }

    return (tokens);
}

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

std::string	ft_tolower(std::string str)
{
	for (size_t i = 0; i < str.size(); i++)
	{
		str[i] = std::tolower(str[i]);
	}
	return (str);
}

std::string	add_to_modestring(std::string const& str, std::string const& mode)
{
	size_t	i = 0;
	bool	found = false;

	while (i < str.size() && str[i] != mode[0])
		++i;
	if (i < str.size() && str[i] == mode[0])
	{
		found = true;
		++i;
	}
	while (i < str.size() && std::isalpha(str[i]))
		++i;
	
	if (found == true)
		return (str.substr(0, i) + mode[1] + str.substr(i));
	return (str + mode);
}

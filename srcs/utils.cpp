#include "../includes/header.hpp"

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
		str[i] = std::tolower(str[i]);
	return (str);
}

std::vector<std::string> split2(const std::string &s, const std::string &delims)
{
		std::vector<std::string> tokens;
		size_t start = 0;
		size_t end = 0;

		while (start < s.length())
		{
			// Skip delimiters
			while (start < s.length() && delims.find(s[start]) != std::string::npos)
				start++;

			if (start >= s.length())
				break;

			// Find next delimiter
			end = start;
			while (end < s.length() && delims.find(s[end]) == std::string::npos)
				end++;

			// Extract token
			std::string token = s.substr(start, end - start);

			// Trim trailing \r\n
			token.erase(token.find_last_not_of("\r\n") + 1);

			tokens.push_back(token);
			start = end;
		}

		return tokens;
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

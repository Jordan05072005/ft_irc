#ifndef HEADER_HPP
# define HEADER_HPP

# include <iostream> // cout, cerr
# include <cstdlib> // atoi
# include <cstdio> // sprintf
# include <cctype> // isdigit, isalpha, tolower
# include <sys/socket.h> // socket, listen, bind, recv, accept, setsockopt, struct sockaddr
# include <fcntl.h> // fcntl, F_SETFL, O_NONBLOCK
# include <unistd.h> // close
# include <poll.h> // poll, struct pollfd
# include <netinet/in.h> // struct sockaddr_in, AF_INET
# include <arpa/inet.h> // htons
# include <vector>
# include <map>
# include <string>
# include <sstream>
# include <ctime>
# include <signal.h>

std::vector<std::string>	split(const std::string &s, char delimiter);
void						setup_signals(void);
std::string					ft_tolower(std::string str);
std::string					add_to_modestring(std::string const& str, std::string const& mode);
# include "Client.hpp"
# include "Channel.hpp"
# include "Server.hpp"

#endif

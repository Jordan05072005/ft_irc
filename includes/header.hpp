#ifndef HEADER_HPP
# define HEADER_HPP

# include <iostream> // cout, cerr
# include <cstdlib> // atoi
# include <cstdio> // sprintf
# include <cctype> // isdigit, isalpha, tolower
# include <cerrno> // errno
# include <sys/socket.h> // socket, listen, bind, recv, accept, setsockopt, struct sockaddr
# include <fcntl.h> // fcntl, F_SETFL, O_NONBLOCK
# include <unistd.h> // close
# include <poll.h> // poll, struct pollfd
# include <netinet/in.h> // struct sockaddr_in, AF_INET
# include <netinet/tcp.h> // TCP_NODELAY, TCP_QUICKACK
# include <arpa/inet.h> // htons
# include <vector> // std:vector
# include <string> // std::string
# include <cstring> // memset
# include <sstream> // std::sstream
# include <ctime> // time_t, time
# include <csignal> // signal, sigaction
# include <algorithm> // find

std::vector<std::string>	split(const std::string &s, char delimiter);
std::vector<std::string>	split2(const std::string &s, const std::string &delims);
void						setup_signals(void);
std::string					ft_tolower(std::string str);
std::string					add_to_modestring(std::string const& str, std::string const& mode);
std::string 				convertTimeStr(time_t t);

typedef struct s_warn
{
	std::string* nick;
	int lvl;
}				t_warn;

# include "Bot.hpp"
# include "Client.hpp"
# include "Channel.hpp"
# include "Server.hpp"

#endif

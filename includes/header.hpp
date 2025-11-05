#ifndef HEADER_HPP
# define HEADER_HPP

# include <iostream> // cout, cerr
# include <cstdlib> // atoi
# include <cctype> // isdigit
# include <sys/socket.h> // socket, listen, bind, recv, accept, setsockopt, struct sockaddr
# include <fcntl.h> // fcntl, F_SETFL, O_NONBLOCK
# include <unistd.h> // close
# include <poll.h> // poll, struct pollfd
# include <netinet/in.h> // struct sockaddr_in, AF_INET
# include <arpa/inet.h> // htons
# include <vector>
# include <string>
# include <sstream>
# include <ctime>
# include <signal.h>

std::vector<std::string>	split(const std::string &s, char delimiter);
void						setup_signals(void);

# include "Client.hpp"
# include "Channel.hpp"
# include "Server.hpp"

#endif

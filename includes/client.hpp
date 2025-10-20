#ifndef CLIENT_HPP
# define CLIENT_HPP

#include <sys/types.h>
#include <netinet/in.h>
#include <iostream>
#include <fcntl.h>   // pour fcntl, F_SETFL, O_NONBLOCK
#include <unistd.h>
#include <vector>
#include <poll.h>
#include <string>


class client {
	private:
		int fd;
		std::string name;
		sockaddr_in addr;
};

#endif
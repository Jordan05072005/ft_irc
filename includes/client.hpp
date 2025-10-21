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
		int _fd;
		std::string _name;
		sockaddr_in _addr;
		socklen_t _len;
		std::string _buff;
	public:
		client();
		client(int fd, sockaddr_in addr, socklen_t len);
		client(const client& cpy);
		client& operator=(const client& cpy);
		~client();

		int getfd();
		void setfd(int fd);
		std::string& getbuf();
		void setbuf(char *buf, int oct);
};

#endif
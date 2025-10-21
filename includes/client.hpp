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
#include <stdlib.h>



class client {
	private:
		int _fd;
		sockaddr_in _addr;
		socklen_t _len;
		std::string _buff;
		std::string _nick;
		bool	_login;
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
		std::string& getnick();
		void setnick(char *nick);
		bool getlogin();
		void setlogin(bool e);
};

#endif
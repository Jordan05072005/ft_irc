#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "header.hpp"

class Client
{
	private:
		Client(void);

		int 		_fd;
		sockaddr_in _addr;
		socklen_t 	_len;
		std::string _buff;
		std::string _nick;
		bool		_login;

	public:
		Client(int fd, sockaddr_in addr, socklen_t len);
		Client(const Client& cpy);
		Client& operator=(const Client& cpy);
		~Client(void);

		int				getFd(void);
		void			setFd(int fd);
		std::string&	getBuf(void);
		void 			setBuf(char *buf, int oct);
		std::string& 	getNick(void);
		void 			setNick(std::string& nick);
		bool 			getLogin();
		void 			setLogin(bool e);
};

#endif

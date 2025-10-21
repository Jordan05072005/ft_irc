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

		int				getfd(void);
		void			setfd(int fd);
		std::string&	getbuf(void);
		void 			setbuf(char *buf, int oct);
		std::string& 	getnick(void);
		void 			setnick(std::string& nick);
		bool 			getlogin();
		void 			setlogin(bool e);
};

#endif

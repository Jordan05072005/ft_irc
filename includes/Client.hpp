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
		std::string _ident;
		std::string _realname;
		std::string	_host;
		int		_etat; // etat = 1 -> login, etat = 2 -> register

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
		int 			getEtat();
		void 			setEtat(int e);
		std::string&	getIdent();
		void 					setIdent(std::string& indent);
		std::string&	getRealName();
		void 					setRealName(std::string& name);
		std::string&	getHost();
		void 					setHost(std::string& host);
};

#endif

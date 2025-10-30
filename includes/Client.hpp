#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "header.hpp"

class Client
{
	private :
		Client(void); // canonical

		//? enlever fd et len, inutiles ?, si oui rectifier Server::delClient()
		int 		_fd; // client socket
		sockaddr_in _addr; // données du client récupérées liées au socket client
		socklen_t 	_len; // taille de sockaddr_in

		std::string _buff;
		std::string _nick;
		std::string _ident;
		std::string _realname;
		std::string	_host;
		int			_etat; // etat = 1 -> login, etat = 2 -> register

	public :
		Client(int fd, sockaddr_in addr, socklen_t len);
		Client(const Client& cpy); // canonical
		Client& operator=(const Client& cpy); // canonical
		~Client(void); // canonical

		int				getFd(void)const;
		void			setFd(int fd);
		const std::string&	getBuf(void)const;
		void 			setBuf(char *buf, int oct);
		const std::string& 	getNick(void)const;
		void 			setNick(std::string& nick);
		int 			getEtat(void)const;
		void 			setEtat(int e);
		const std::string&	getIdent(void)const;
		void 			setIdent(std::string& indent);
		const std::string&	getRealName(void)const;
		void 			setRealName(std::string& name);
		const std::string&	getHost(void)const;
		void 			setHost(std::string& host);
};

#endif

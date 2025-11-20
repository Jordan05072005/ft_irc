#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "header.hpp"

class Channel;

class Client
{
	private :
		Client(void); // canonical
		Client(const Client& cpy); // canonical
		Client& operator=(const Client& cpy); // canonical

		int 		_fd; // client socket
		sockaddr_in _addr; // données du client récupérées liées au socket client
		socklen_t 	_len; // taille de sockaddr_in

		std::string	_buff;

		std::string _nick;
		std::string _ident;
		std::string _realname;
		std::string	_host;
		std::string	_serv;

		std::vector<Channel*>	_channel;

		int						_state; // state = 1 -> login, state = 2 -> register

	public :
		Client(int fd, sockaddr_in addr, socklen_t len); // canonical
		~Client(void); // canonical

		int						getFd(void) const;
		void					setFd(int fd);

		std::string const&		getBuf(void) const;
		void 					addBuf(char *buf, int len);
		void 					resetBuf(void);

		std::string const&		getNick(void) const;
		void 					setNick(std::string& nick);
		std::string const&		getIdent(void) const;
		void 					setIdent(std::string& indent);
		std::string const&		getRealName(void) const;
		void 					setRealName(std::string& name);
		std::string const&		getHost(void) const;
		void	 				setHost(std::string& host);
		std::string const&		getServ(void) const;
		void	 				setServ(std::string& host);

		std::vector<Channel*>&	getChannels(void);
		void					addChannel(Channel* channel);
		void					removeChannel(std::string const& name);
		void					removeAllChannels(void);

		int 					getState(void) const;
		void 					setState(int e);
};

#endif

#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "header.hpp"

class Channel;

class Client
{
	private :
		int 		_fd; // client socket
		sockaddr_in _addr; // données du client récupérées liées au socket client
		socklen_t 	_len; // taille de sockaddr_in
		std::string	_buff;
		std::string _nick;
		std::string _ident;
		std::string _realname;
		std::string	_host;
		std::vector<Channel>	_channel;
		int			_state; // state = 1 -> login, state = 2 -> register

	public :
		Client(void); // canonical
		Client(int fd, sockaddr_in addr, socklen_t len); // canonical
		Client(const Client& cpy); // canonical
		Client& operator=(const Client& cpy); 
		~Client(void); // canonical

		int							getFd(void) const;
		void						setFd(int fd);

		std::string const&			getBuf(void) const;
		void 						setBuf(char *buf, int oct);

		const std::string&			getNick(void) const;
		void 						setNick(std::string& nick);
		const std::string&			getIdent(void) const;
		void 						setIdent(std::string& indent);
		const std::string&			getRealName(void) const;
		void 						setRealName(std::string& name);
		const std::string&			getHost(void) const;
		void 						setHost(std::string& host);

		void						addChannel(Channel& name);
		void						removeAllChannels(void);

		int 						getState(void) const;
		void 						setState(int e);
};

#endif

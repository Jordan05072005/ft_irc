#ifndef FT_IRC_HPP
#define  FT_IRC_HPP


# include "./client.hpp"

class ft_irc {
	private:
		std::vector<pollfd> _fds;
		std::vector<client> _clients; 
		int	_port_serv;
		std::string _password;
		sockaddr_in	_addr;
		void	bindAndListen(const sockaddr_in &addr);
		void	delClient(int index);
	public:
		ft_irc();
		ft_irc(int port, char *password);
		void initSev();
		void startSev();
		void requeteGestion(client& client);
};


#endif

#ifndef FT_IRC_HPP
#define  FT_IRC_HPP


# include "./client.hpp"

class ft_irc {
	private:
		std::vector<pollfd> _fds;
		std::vector<client> clients; 
		int	_port_serv;
		sockaddr_in _addr;
		void	bindAndListen(const sockaddr_in &addr);
	public:
		ft_irc();
		ft_irc(int port);
		void initSev();
		void startSev();
};


#endif

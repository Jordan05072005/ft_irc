#ifndef FT_IRC_HPP
#define  FT_IRC_HPP

#include <sys/types.h>
#include <netinet/in.h>
#include <iostream>
#include <fcntl.h>   // pour fcntl, F_SETFL, O_NONBLOCK
#include <unistd.h>

class ft_irc {
	private:
		int _serv_fd;
		int	_port_serv;
		sockaddr_in addr;
		void	bindAndListen(const sockaddr_in &addr);
	public:
		ft_irc();
		ft_irc(int port);
		void initSev();

};


#endif

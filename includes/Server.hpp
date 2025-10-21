#ifndef Server_HPP
# define  Server_HPP

# include "header.hpp"

class Server
{
	private:
		Server(void);
		void initServ(void);

		std::vector<pollfd> _fds;
		std::vector<Client> _clients; 

		int	_port_serv;
		std::string _password;

		sockaddr_in	_addr;

		void	bindAndListen(const sockaddr_in &addr);
		void	delClient(int index);

	public:
		Server(int port, char *password);

		void startServ(void);
		void requeteGestion(Client& client);
};


#endif

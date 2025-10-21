#ifndef SERVER_HPP
# define  SERVER_HPP

# include "header.hpp"

class Client;

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
		int checkPass(Client& client, int *i, std::vector<std::string> mess);
	public:
		Server(int port, char *password);

		void startServ(void);
		void requeteGestion(Client& client, int *i);
};


#endif

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

		int			_port_serv;
		std::string	_password;

		sockaddr_in	_addr;

		void	bindAndListen(sockaddr_in const& addr);
		void	delClient(int index);
		void	requestHandler(Client& client, int* i);
		int		checkPass(Client& client, int* i, std::vector<std::string> mess);

	public:
		Server(int port, std::string const& password);

		void startServ(void);
};


#endif

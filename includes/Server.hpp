#ifndef SERVER_HPP
# define  SERVER_HPP

# include "header.hpp"

class Client;
class Server;

typedef struct cmd{
	std::string name;
	int (Server::*pars)(Client&, std::vector<std::string>&);
	int	etat;
}	t_cmd;

class Server
{
	private:
		Server(void);
		std::vector<pollfd> _fds;
		std::vector<Client> _clients; 
		int			_port_serv;
		std::string	_password;
		sockaddr_in	_addr;
		std::vector<t_cmd> _cmd;

		void	initServ(void);
		void	bindAndListen(sockaddr_in const& addr);
		void	delClient(int index);
		int		requestHandler(Client& client);
		void	send_error(std::string err, Client& client, std::string body);
		int		errorEtat(int etat, std::string& cmd, Client &client);
		int		uniqueNick(std::string &nick);
		int		checkPass(Client& client, std::vector<std::string>& mess);
		int		checkNick(Client& client, std::vector<std::string>& mess);
		int		checkUser(Client& client, std::vector<std::string>& mess);
	public:
		Server(int port, std::string const& password);
		void startServ(void);
};


#endif

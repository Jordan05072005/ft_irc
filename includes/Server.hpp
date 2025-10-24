#ifndef SERVER_HPP
# define  SERVER_HPP

# include "header.hpp"


class Client;


typedef struct cmd
{
	std::string name;
	int (Server::*pars)(Client&, std::vector<std::string>&);
	int	etat;
}				t_cmd;


class Server // TODO : forme canonique
{
	private :
		Server(void);
		
		int			_port_serv; // port donné au constructeur
		std::string	_password; // mdp donné au constructeur
		
		std::vector<pollfd> _fds; // tableau de structre, contient sockets et évènement à surveiller
		std::vector<Client> _clients; // tableau de classe, gestionnaire des données client
		
		sockaddr_in	_addr; // données à mettre dans le socket principal
		std::vector<t_cmd> _cmd;
		// std::vector<Channel> channel;

		void	initServ(void);
		void	bindAndListen(sockaddr_in const& addr);
		void	run(void);
		void	delClient(int index);
		int		requestHandler(Client& client);
		void	send_error(std::string err, Client& client, std::string body);
		void	send_mess(std::string channel, std::string cmd, std::string mess, Client& c);
		int		errorEtat(int etat, std::string& cmd, Client &client);
		int		uniqueNick(std::string &nick);
		int		checkPass(Client& client, std::vector<std::string> mess);
		int		checkNick(Client& client, std::vector<std::string>& mess);
		int		checkUser(Client& client, std::vector<std::string>& mess);
		int		checkQuit(Client& client, std::vector<std::string>& mess);
	public:
		Server(int port, std::string const& password);

};

#endif

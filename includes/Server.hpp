#ifndef SERVER_HPP
# define  SERVER_HPP

# include "header.hpp"


class Client;
class Server;

typedef struct cmd
{
	std::string name;
	int (Server::*pars)(Client&, std::vector<std::string>&);
	int	etat;
}				t_cmd;

class Server
{
	private :
		Server(void); // canocical
		Server(Server const& copy); // canonical
		Server&	operator=(Server const& other); // canonical
		int			_port_serv; // port donné au constructeur
		std::string	_password; // mdp donné au constructeur
		std::vector<pollfd> _fds; // tableau de structre, contient sockets et évènement à surveiller
		std::vector<Client> _clients; // tableau de classe, gestionnaire des données client
		sockaddr_in	_addr; // données à mettre dans le socket principal
		std::vector<t_cmd> _cmd;
		std::vector<Channel> _channel;
		
		void	initServ(void);
		void	bindAndListen(sockaddr_in const& addr);
		void	run(void);
		void	delClient(int index);
		int		requestHandler(Client& client);
		void	sendMessLocal(std::string err, std::string cmd, Client& client, std::string body);
		void	sendMessGlobal(std::string channel, std::string cmd, std::string mess, Client& c);
		int		errorEtat(int etat, std::string& cmd, Client &client);
		int		uniqueNick(std::string &nick);
		int		checkChannel(std::string &name);
		int		getChannel(std::string &name);
		int		checkPass(Client& client, std::vector<std::string>& mess);
		int		checkNick(Client& client, std::vector<std::string>& mess);
		int		checkUser(Client& client, std::vector<std::string>& mess);
		int		checkCap(Client& client, std::vector<std::string>& mess);
		int		checkQuit(Client& client, std::vector<std::string>& mess);
		int		checkKick(Client& client, std::vector<std::string>& mess);

	public :
		Server(int port, std::string const& password); // canonical
		~Server(void); // canonical

};

#endif

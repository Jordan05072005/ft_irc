#ifndef SERVER_HPP
# define  SERVER_HPP

# include "header.hpp"
# define POOL_TIME 1000

class Client;
class Server;

typedef struct cmd
{
	std::string name;
	int (Server::*pars)(Client&, std::vector<std::string>&);
	int	state;
}				t_cmd;

class Server
{
	private :
		Server(void); // canocical
		// Server(Server const& copy); // canonical
		// Server&	operator=(Server const& other); // canonical
		
		bool					_init;
		int						_port_serv; // port donné au constructeur
		std::string				_password; // mdp donné au constructeur
		
		std::vector<pollfd> 	_fds; // tableau de structre, contient sockets et évènement à surveiller
		std::vector<Client*> 	_clients; // tableau de classe, gestionnaire des données client
		
		sockaddr_in				_addr; // données à mettre dans le socket principal
		std::vector<t_cmd>		_cmd;
		std::vector<Channel*>	_channel;
		/*
			!!!!!!!!

			!std::vector<Channel* / Client*> au lieu de std::vector<Channel / Client> car adresses changent à chaque push_back
			!donc création des Channels / Clients avec new

			!penser a new et delete les channels/clients du tableau suivant les mouvements des clients

			!!!!!!!!
		*/
		void	initServ(void);
		void	bindAndListen(sockaddr_in const& addr);
		void	run(void);
		int		requestHandler(Client& client);
	
		int		checkCap(Client& client, std::vector<std::string>& mess);
		int		checkPass(Client& client, std::vector<std::string>& mess);
		int		checkNick(Client& client, std::vector<std::string>& mess);
		int		checkUser(Client& client, std::vector<std::string>& mess);
		int		checkQuit(Client& client, std::vector<std::string>& mess);
		int		checkKick(Client& client, std::vector<std::string>& mess);
		int		checkInvite(Client& client, std::vector<std::string>& mess);
		int		checkTopic(Client& client, std::vector<std::string>& mess);
		int		checkJoin(Client& client, std::vector<std::string>& mess);
		int		checkMode(Client& client, std::vector<std::string>& mess);

		int		checkUniqueNick(std::string& nick);
		int		checkExistClient(std::string& nick);
		int		checkExistChannel(std::string& name);

		int		errorState(int state, std::string& cmd, Client& client);
		void	sendMessLocal(std::string err, std::string cmd, Client& client, std::string body);
		void	sendMessGlobal(std::string channel, std::string cmd, std::string mess, Client& c);
	
		int		getIndexChannel(std::string& name);
		Client&	getClient(std::string& nick);

		void	delClient(int index);
		void	delInvite(void);
		
	public :
		//Server(int port, std::string const& password); // canonical
		void 			init(int port, std::string const& password);
		static Server&	getInstance(void);
		void			closeAll();
		~Server(void) // canonical

};

#endif

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
	int	etat;
}				t_cmd;

class Server
{
	private :
		Server(void); // canocical
		Server(Server const& copy); // canonical
		Server&	operator=(Server const& other); // canonical
		bool		_init;
		int			_port_serv; // port donné au constructeur
		std::string	_password; // mdp donné au constructeur
		std::vector<pollfd> _fds; // tableau de structre, contient sockets et évènement à surveiller
		std::vector<Client> _clients; // tableau de classe, gestionnaire des données client
		sockaddr_in	_addr; // données à mettre dans le socket principal
		std::vector<t_cmd> _cmd;
		std::vector<Channel> _channel;

		void		initServ(void);
		void		bindAndListen(sockaddr_in const& addr);
		void		run(void);
		void		delClient(int index);
		int			requestHandler(Client& client);
		void		sendMessLocal(std::string err, std::string cmd, Client& client, std::string body);
		void		sendMessGlobal(std::string channel, std::string cmd, std::string mess, Client& c);
		int			errorEtat(int etat, std::string& cmd, Client &client);
		int			uniqueNick(std::string &nick);
		int			checkExistChannel(std::string &name);
		int			checkExistClient(std::string &nick);
		int			getChannel(std::string &name);
		Client&	getClient(std::string &nick);
		void		delInvite();
		int			checkPass(Client& client, std::vector<std::string>& mess);
		int			checkNick(Client& client, std::vector<std::string>& mess);
		int			checkUser(Client& client, std::vector<std::string>& mess);
		int			checkCap(Client& client, std::vector<std::string>& mess);
		int			checkQuit(Client& client, std::vector<std::string>& mess);
		int			checkKick(Client& client, std::vector<std::string>& mess);
		int			checkInvite(Client& client, std::vector<std::string>& mess);
		int			checkTopic(Client& client, std::vector<std::string>& mess);

	public :
		void init(int port, std::string const& password);
		static Server& getInstance();
		void		closeAll();
		~Server(void); // canonical
};

#endif

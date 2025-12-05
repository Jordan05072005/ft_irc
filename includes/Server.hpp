#ifndef SERVER_HPP
# define  SERVER_HPP

# include "header.hpp"
// # define POOL_TIME 1000

class Client;
class Bot;
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
		Server(Server const& copy); // canonical
		Server&	operator=(Server const& other); // canonical
		
		bool					_init;
		bool 					_close;
		int						_port_serv;
		std::string				_password;
		sockaddr_in				_addr; // data of server's machine socket
		
		std::vector<pollfd> 	_fds; // sockets to look for communication
		std::vector<Client*> 	_clients;
		std::vector<Channel*>	_channel;
		std::vector<Bot*> 		_bot;
		std::vector<t_cmd>		_cmd;

		void		initServ(void);
		void		bindAndListen(sockaddr_in const& addr);
		void		run(void);
		int			requestHandler(Client& client);
	
		int			checkCap(Client& client, std::vector<std::string>& mess);
		int			checkPass(Client& client, std::vector<std::string>& mess);
		int			checkNick(Client& client, std::vector<std::string>& mess);
		int			checkUser(Client& client, std::vector<std::string>& mess);
		int			checkQuit(Client& client, std::vector<std::string>& mess);
		int			checkKick(Client& client, std::vector<std::string>& mess);
		int			checkInvite(Client& client, std::vector<std::string>& mess);
		int			checkTopic(Client& client, std::vector<std::string>& mess);
		int			checkJoin(Client& client, std::vector<std::string>& mess);
		int			checkMode(Client& client, std::vector<std::string>& mess);
		int			checkPart(Client& client, std::vector<std::string>& mess);
		int			checkNotice(Client& client, std::vector<std::string>& mess);
		int			checkList(Client& client, std::vector<std::string>& mess);
		int			checkPrivmsg(Client& client, std::vector<std::string>& mess);
		int			checkNames(Client& client, std::vector<std::string>& mess);
		int			checkWho(Client& client, std::vector<std::string>& mess);
		int			checkWhois(Client& client, std::vector<std::string>& mess);
		int			checkHelp(Client& client, std::vector<std::string>& mess);
		int			checkPing(Client& client, std::vector<std::string>& mess);
		int			checkRules(Client& client, std::vector<std::string>& mess);
		int			checkStats(Client& client, std::vector<std::string>& mess);
		int			checkNote(Client& client, std::vector<std::string>& mess);

		int			checkExistClient(std::string const& nick);
		int			checkExistChannel(std::string const& name);
		int			autorisedNick(std::string& name);
		int			autorisedIdent(std::string& name);
		int			autorisedRealName(std::string& name);

		int			errorState(int state, std::string const& cmd, Client& client);
		void		sendMessLocal(std::string const& err, std::string const& cmd, Client const& client, std::string const& body);
		void		sendMessUser(Client const& s, Client const& r, std::string const& cmd, std::string const& body);
		void		sendMessGlobal(std::string const& cmd, std::string const& mess, Client const& c);
		void		sendMessChannel(std::string const& channel, std::string const& argm, std::string const& mess, int sendme, Client& c);
		void		sendMessBot(Bot& b, Client const& c, std::string cmd, const std::string& mess);
	
		int			getIndexChannel(std::string const& name);
		int			getIndexClient(std::string const& nick);

		void		delClient(int index);
		void		delInvite(void);
		void		delAllChannelClient(Client& client, std::string& cmd, std::string mess);

		std::string	createStringChannels(void) const;
		
	public :
		void 			init(int port, std::string const& password);
		static Server&	getInstance(void);
		void			closeAll(void);
		void			close_serv(void);
		virtual ~Server(void); // canonical

};

#endif

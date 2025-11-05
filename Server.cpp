/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jguaglio <guaglio.jordan@gmail.com>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/21 13:48:33 by jguaglio          #+#    #+#             */
/*   Updated: 2025/10/21 13:48:33 by jguaglio         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "includes/header.hpp"

static t_cmd init_cmd(std::string n, int (Server::*func)(Client&, std::vector<std::string>&), int state)
{
	t_cmd p;

	p.name = n;
	p.pars = func;
	p.state = state;
	return (p);
}

// initialisation de struct pollfd pour chaque socket
static pollfd init_pollfd(int fd, short events, short revents)
{
	pollfd p;

	p.fd = fd; // fd ouvert par le socket
	p.events = events; // évènements à surveiller, POLLIN = true/false if must read in fd
	p.revents = revents; // activation ou non des évènements, le kernell le remplie, 0 par défaut

	return (p);
}

Server::Server(void){}

Server& Server::getInstance(void)
{
	static Server instance;
	return instance;
}

void	Server::init(int port, std::string const& password)
{
	if (!this->_init)
	{
		this->_port_serv = port;
		this->_init = true;
		this->_password.assign(password);
		this->initServ();

		// available commands when not login nor registered
		this->_cmd.push_back(init_cmd("CAP", &Server::checkCap, 0));
		this->_cmd.push_back(init_cmd("PASS", &Server::checkPass, 0));

		// available commands when login and registered
		this->_cmd.push_back(init_cmd("NICK", &Server::checkNick, 1));
		this->_cmd.push_back(init_cmd("USER", &Server::checkUser, 1));
		this->_cmd.push_back(init_cmd("QUIT", &Server::checkQuit, 1));

		// available commands when registered
		this->_cmd.push_back(init_cmd("KICK", &Server::checkKick, 2));
		this->_cmd.push_back(init_cmd("INVITE", &Server::checkInvite, 2));	
		this->_cmd.push_back(init_cmd("TOPIC", &Server::checkTopic, 2));
		this->_cmd.push_back(init_cmd("JOIN", &Server::checkJoin, 2));

		this->run();
	}

	return ;
}

Server::~Server(void){}

void Server::closeAll(){
	for (size_t i = 0; i < this->_fds.size(); i++){
		if (i == 0)
			close(this->_fds[0].fd);
		else{
			this->delClient(i);
		}
	}
}


/*-----------------------------------------------------------------------------------------------*/

// création socket de base pour connexions client
void Server::initServ(void)
{
	int opt = 1; //? pk 1

	this->_fds.push_back(init_pollfd(socket(AF_INET, SOCK_STREAM, 0), POLLIN, 0)); // socket(ipv4, SOCK_STREAM = oblige listen et accept, Protocole par défaut TCP)
	if (this->_fds[0].fd == -1)
		throw std::runtime_error("Error: server socket creation");
	this->_addr.sin_family = AF_INET; // config pour ipv4
	this->_addr.sin_port = htons(this->_port_serv); // convertis le port pour la struct
	this->_addr.sin_addr.s_addr = INADDR_ANY; // connexion possible à partir de toutes interfaces de la machine herbergeant le serv
	fcntl(this->_fds[0].fd, F_SETFL, O_NONBLOCK); // fd/socket non bloquant

	// peut reutiliser le port pris sur toutes interfaces (données du socket) immediatement si serv crash sans fermer socket
	setsockopt(this->_fds[0].fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	this->bindAndListen(this->_addr);

	return ;
}

// association données du socket au socket en lui-même
// puis mise sur écoute du socket
void Server::bindAndListen(sockaddr_in const& addr)
{
	if (bind(this->_fds[0].fd, (sockaddr*)&addr, sizeof(addr)) < 0)
		throw std::runtime_error("Error: binding server socket");
	if (listen(this->_fds[0].fd, SOMAXCONN) < 0) // SOMAXCONN = taille max file d'attente
		throw std::runtime_error("Error: listening on server socket");

	return ;
}

// boucle infinie, serveur tournant
void Server::run(void)
{
	int					err;

	sockaddr_in			client_addr; // données du client récupérées liées au socket client
	socklen_t			client_len = sizeof(client_addr);
	int 				client_fd; // socket client

	char 				buf[512];
	int 				oct; // nombre d'octets lus renvoyés par recv

	while (1)
	{
		err = poll(this->_fds.data(), this->_fds.size(), POOL_TIME); // poll(pointeur sur tableau, taille tableau, timeout)
		if (err < 0)
			throw std::runtime_error("Error: poll");
		this->delInvite();
		setup_signals();

		// traitement server socket
		if (this->_fds[0].revents & POLLIN) // & opération binaire, indique si le flag POLLIN est "activé", renvoie 0 ou POLLIN
		{
			if (this->_clients.size() >= 1)
				std::cout << "befre " << this->_clients[0].getState()<< std::endl;
			client_fd = accept(this->_fds[0].fd, (sockaddr*)&client_addr, &client_len); // renvoie le socket client et stocke ses données dans une struct
			if (client_fd < 0)
				throw std::runtime_error("Error: acceptance client socket connection");
			fcntl(client_fd, F_SETFL, O_NONBLOCK); // fd/socket non bloquant
			this->_clients.push_back(Client(client_fd, client_addr, client_len)); // ajout des données client au tableau
			std::cout << "after " << this->_clients[0].getState()<< std::endl;
			this->_fds.push_back(init_pollfd(client_fd, POLLIN, 0)); // ajout socket client au tableau
			this->_fds[0].revents = 0; // remise à défaut, évènement non actif
			std::cout << "new" << std::endl;
		}

		// traitement client sockets
		for (int i = 1; i < (int)this->_fds.size(); i++)
		{
			if (this->_fds[i].revents & POLLIN)
			{
				oct = recv(this->_fds[i].fd, buf, sizeof(buf), 0); // remplis le buffer en lisant sur le client socket, 0 parce que pas de flag
				if (oct <= 0)
				{
					this->delClient(i--); // envoie i puis enlève 1
					continue; // revient au début de boucle for
				}

				this->_clients[i - 1].setBuf(buf, oct);

				// partie test récuperation de requête
				std::cout << this->_clients[i - 1].getBuf() << std::endl;

				if (this->requestHandler(this->_clients[i - 1]))
				{
					this->delClient(i--);
					continue;
				}
				// send(this->_fds[i].fd, buf, oct, 0); // envoie une réponse à la requête client
				this->_fds[i].revents = 0; // remise à défaut, évènement non actif
			}
			else if (this->_fds[i].revents & (POLLERR | POLLHUP | POLLNVAL))
				this->delClient(i--);
		}
	}

	return ;
}

// gestion des données reçues
int Server::requestHandler(Client& client)
{
	std::vector<std::string> mess = split(client.getBuf(), ' ');
	if (mess.size() <= 0)
		return 0;
	for (size_t i = 0; i < this->_cmd.size(); i++)
	{
		if (mess[0] == this->_cmd[i].name)
		{
			if (this->_cmd[i].state > client.getState())
				return (this->errorState(client.getState(), mess[0], client));
			return (this->*(_cmd[i].pars))(client, mess);
		}
	}
	return (this->sendMessLocal("462", mess[0], client, "Unknown command"), 0);
}


/*-----------------------------------------------------------------------------------------------*/


int	Server::checkCap(Client& client, std::vector<std::string>& mess)
{
	std::string message;

	std::cout << "here" << std::endl;

	message = ":irc CAP * LS :";
	send(client.getFd(), message.c_str(), message.size(), 0);
	(void)mess;
	return 0;
}

// vérification du mdp
int Server::checkPass(Client& client, std::vector<std::string>& mess)
{
	std::string err;

	if (client.getState() >= 1)
		return (this->sendMessLocal("462", "", client, "Unauthorized command (already registered)"), 0);
	if (mess[1] != this->_password)
		return (this->sendMessLocal("464", "", client, "Incorrect password"), 1);

	client.setState(client.getState() + 1);

	return (0);
}

int Server::checkNick(Client& client, std::vector<std::string>& mess)
{
	if (!client.getNick().empty())
		return (this->sendMessLocal("462", "", client, "Unauthorized command (already registered)"), 0);
	if (mess.size() == 1)
		return (this->sendMessLocal("431", "", client, "No nickname given"), 0);
	if (this->checkUniqueNick(mess[1]))
		return (this->sendMessLocal("433", "", client, "Nickname is already in use"), 0);
	client.setNick(mess[1]);
	if (!client.getIdent().empty())
	{
		client.setState(client.getState() + 1);
		this->sendMessLocal("001", "", client, "Welcome to the IRC Network");
	}
	
	return 0;
}

int Server::checkUser(Client& client, std::vector<std::string>& mess)
{
	std::string realname;

	if (!client.getRealName().empty() || !client.getIdent().empty())
		return (this->sendMessLocal("462", "", client, "Unauthorized command (already registered)"), 0);
	if (mess.size() < 5) // TODO : reduire a 1 if
		return (this->sendMessLocal("461", "", client, "Not enough parameters"), 0);
	if (mess[4][0] != ':')
		return (this->sendMessLocal("461", "", client, "Not enough parameters"), 0);
	client.setIdent(mess[1]);
	for (int i = 4; i < (int)mess.size(); i++)
	{
		realname += mess[i];
	}
	client.setRealName(realname.erase(0, 1));
	if (!client.getNick().empty())
	{
		client.setState(client.getState() + 1);
		this->sendMessLocal("001", "", client, "Welcome to the IRC Network");
	}
	return 0;
}

int Server::checkQuit(Client& client, std::vector<std::string>& mess)
{
	std::string message;
	if (mess.size() == 1)
		message = "Client Quit";
	else
	{
		if (mess[1][0] != ':')
			message = mess[1];
		else
		{
			for (int i = 1; i < (int)mess.size(); i++)
			{
				message += mess[i];
			}
		}
	}
	if (client.getState() < 2)
		return (1);
	this->sendMessGlobal("", mess[0], message, client);
	return (1);
}

//KICK <channel> <user> [<comment>]
int	Server::checkKick(Client& client, std::vector<std::string>& mess)
{
	std::string message;
	Channel channel;

	if (mess.size() < 3)
		return (this->sendMessLocal("461", mess[0], client, "Not enough parameters"), 0);
	if (!this->checkExistChannel(mess[1]))
		return (this->sendMessLocal("403", mess[1], client, "No such channel"), 0);
	channel = this->_channel[this->getIndexChannel(mess[1])];
	if (!channel.checkUser(client.getNick()))
		return (this->sendMessLocal("442", mess[1], client, "You're not on that channel"), 0);
	if (!channel.checkOperator(client.getNick()))
		return (this->sendMessLocal("482", mess[1], client, "You're not channel operator"), 0);
	if (!channel.checkUser(mess[2]))
		return (this->sendMessLocal("441", mess[2] + " " + mess[1], client, "They aren't on that channel"), 0);
	channel.removeUser(mess[2]);
	if (mess.size() == 3)
		message == "";
	else if (mess[3][0] == ':'){
		mess[3].erase(0, 1);
		for (size_t i = 3; i < mess.size(); i++)
			message += (mess[i] + " ");
		std::cout << message << std::endl;
	}
	else
		message = mess[3];
	this->sendMessGlobal(mess[1] + " " + mess[2], mess[0], message, client);
	return (0);
}
// :<kicker_nick>!<user>@<host> KICK <channel> <target_nick> :<comment>
// :Alice!~alice@127.0.0.1 KICK #42school johon :Trop de spam
// :server 461 <nick> KICK :Not enough parameters

//INVITE <nickname> <channel>
int Server::checkInvite(Client& client, std::vector<std::string>& mess)
{
	Channel channel;

	if (mess.size() < 3)
		return (this->sendMessLocal("461", mess[0], client, "Not enough parameters"), 0);
	if (!this->checkExistClient(mess[1]))
		return (this->sendMessLocal("401", mess[1], client, "No such nick/channel"), 0);
	if (!this->checkExistChannel(mess[2]))
		return (this->sendMessLocal("403", mess[2], client, "No such channel"), 0);
	channel = this->_channel[this->getIndexChannel(mess[2])];
	if (!channel.checkUser(client.getNick()))
		return (this->sendMessLocal("442", mess[2], client, "You're not on that channel"), 0);
	if (channel.getOptInviteOnly() && !channel.checkOperator(client.getNick()))
		return (this->sendMessLocal("482", mess[2], client, "You're not channel operator"), 0);
	this->sendMessLocal("341", mess[1] + " " + mess[2], client, "");
	this->sendMessLocal("", mess[0], client, mess[2]);
	channel.addInvite(this->getClient(mess[1]));
	return (0);
}

static std::string converTimeStr(time_t t){

	std::ostringstream oss;
	oss << t;
	std::string time_str = oss.str();
	return (time_str);
}

// TOPIC <Channel> [<newTopic>]
int Server::checkTopic(Client& client, std::vector<std::string>& mess)
{
	Channel channel;
	std::string message;
	t_topic t;

	if (mess.size() < 2 || (mess.size() > 2 && mess[2][0] != ':'))
		return (this->sendMessLocal("461", mess[0], client, "Not enough parameters"), 0);
	if (!this->checkExistChannel(mess[1]))
		return (this->sendMessLocal("403", mess[1], client, "No such channel"), 0);
	channel = this->_channel[this->getIndexChannel(mess[1])];
	if (!channel.checkUser(client.getNick()))
		return (this->sendMessLocal("442", mess[1], client, "You're not on that channel"), 0);
	if (channel.getOptRestrictTopic()
		&& !channel.checkOperator(client.getNick()))
		return (this->sendMessLocal("482", mess[2], client, "You're not channel operator"), 0);
	if (mess.size() == 2)
	{
		if (channel.getTopic().topic.empty())
			return (this->sendMessLocal("33", mess[1], client, "No topic is set"), 0);
		t = channel.getTopic();
		this->sendMessLocal("332", mess[1], client, t.topic);
		return (this->sendMessLocal("333", mess[1] + " " + t.modifBy + " " + converTimeStr(t.time), client, ""), 0);
	}
	for (size_t i = 2; i < mess.size(); i++)
		message += (mess[i] + " ");
	channel.setTopic(message, client.getNick());
	return(this->sendMessGlobal(mess[1], mess[0], message, client), 0);
}

// JOIN [#channel,&channel] [key,key]
int		Server::checkJoin(Client& client, std::vector<std::string>& mess) // TODO : plusieurs channels en même temps
{
	Channel channel;

	if (mess.size() < 2)
		return (this->sendMessLocal("461", mess[0], client, "Not enough parameters"), 0);
	if (mess[1] == "0" && mess.size() == 2)
		return (client.removeAllChannels(), 0);
	if (!this->checkExistChannel(mess[1])){ // debug
		if (this->newChannel(mess[1], client))
			return 0;
	}
	else
		this->addClientChannel(mess[1], client);
	return 0;
	channel = this->_channel[this->getIndexChannel(mess[1])];

	if (channel.getOptInviteOnly() == true)
	{
		std::vector<t_invite> const	invite = channel.getInvite();
		bool						is_invite = false;

		for (int i = 0; i < (int)invite.size(); ++i)
		{
			if (client.getNick() == invite[i].client->getNick())
			{
				is_invite = true;
				break;
			}
		}

		if (is_invite == false)
			return (this->sendMessLocal("473", mess[0], client, "Cannot join channel (+i)"), 0);
	}
	if (mess.size() > 2 && channel.getOptChannelKey() == true && channel.getChannelKey() != mess[2])
		return (this->sendMessLocal("475", mess[0], client, "Cannot join channel (+k)"), 0);

	// TODO : revoir le message qd qqn join
	this->sendMessGlobal(mess[1], mess[0], mess[1], client);

	// TODO : voir pour envoyer des messages privé du serv au client dans un channel
	if (!channel.getTopic().topic.empty())
		this->sendMessLocal("332", "", client, channel.getTopic().topic);
	this->sendMessLocal("353", "= " + mess[1], client, channel.createStringUsers());
	this->sendMessLocal("366", "", client, "End of /NAMES list");
	return(0);
}


/*-----------------------------------------------------------------------------------------------*/


int	Server::checkUniqueNick(std::string &nick)
{
	for (size_t i = 0; i < this->_clients.size(); i++)
	{
		if (this->_clients[i].getNick() == nick)
			return 1;
	}
	return 0;
}

int Server::checkExistClient(std::string& nick)
{
	for (size_t i = 0; i < this->_clients.size(); i++)
	{
		if (this->_clients[i].getNick() == nick)
			return 1;
	}
	return 0;
}

int	Server::checkExistChannel(std::string &name) //? voir #
{
	for (size_t i = 0; i < this->_channel.size(); i++)
	{
		if (this->_channel[i].getName() == name)
			return (1);
	}
	return (0);
}


/*-----------------------------------------------------------------------------------------------*/


int Server::errorState(int state, std::string& cmd, Client &client)
{
	if (state == 0)
		return (this->sendMessLocal("461", "",client, "Password needed"), 1);
	else if (state  == 1)
		this->sendMessLocal("451", cmd, client, "You have not registered");
	return (0);
}
//:ft_irc 451 JOIN :You have not registered

void Server::sendMessLocal(std::string err, std::string cmd, Client& c, std::string body)
{
	std::string err_mess;
	std::string nick;
	std::stringstream ss;

	nick = c.getNick();
	if (nick.empty())
		nick = "*";
	if (err.empty())
		ss << ":" << nick << "!" << (c.getIdent().empty() ? "*" : c.getIdent()) << "@" << c.getHost() << (cmd.empty() ? "" : (" " + cmd)) << " " << nick << " " << (body.empty() ? "" : (": " + body)) << "\r\n";
	else
		ss << ":irc " << err << " " << nick << (cmd.empty() ? "" : (" " + cmd)) << (body.empty() ? "" : (": " + body)) << "\r\n";
	err_mess = ss.str();
	send(c.getFd(), err_mess.c_str(), err_mess.size(), 0);
}

void Server::sendMessGlobal(std::string channel, std::string cmd, std::string mess, Client& c)
{
	std::stringstream ss;
	std::string message;
	int i;
	std::vector<Client> t;

	if (channel.empty())
	{
		ss << ":" << (c.getNick().empty() ? "*" : c.getNick()) << "!~" << (c.getIdent().empty() ? "*" : c.getIdent()) << "@" << c.getHost() << " " << cmd << (mess.empty() ? "" : (": " + mess)) << "\r\n";
		message = ss.str();
		for (size_t i = 0; i < this->_clients.size(); i++)
		{
			send(this->_clients[i].getFd(), message.c_str(), message.size(), 0);
		}
		return ;
	}
	ss << ":" << (c.getNick().empty() ? "*" : c.getNick()) << "!~" << (c.getIdent().empty() ? "*" : c.getIdent()) << "@" << c.getHost() << " " << cmd << " " << channel << (mess.empty() ? "" : (": " + mess)) << "\r\n";
	message = ss.str();
	i = this->getIndexChannel(channel);
	t = this->_channel[i].getUsers();
	for (size_t i = 0; i < t.size(); i++)
		send(t[i].getFd(), message.c_str(), message.size(), 0);
}

// :john!~john@127.0.0.1 PRIVMSG #general :Salut tout le monde ! 
// pour Channel
// :nick!user@host QUIT :Client Quit


/*-----------------------------------------------------------------------------------------------*/


int	Server::getIndexChannel(std::string &name)
{
	for (size_t i = 0; i < this->_channel.size(); i++)
	{
		if (this->_channel[i].getName() == name)
			return (i);
	}
	return (0);
}

Client&	Server::getClient(std::string &nick)
{
	for (size_t i = 0; i < this->_clients.size(); i++)
	{
		if (this->_clients[i].getNick() == nick)
			return (this->_clients[i]);
	}
	return (this->_clients[0]); // trouver solution pr null
}

static int checkcara(std::string& channel){
	for (size_t i = 0; i < channel.size(); i++){
		if (channel[i] < 32 || channel[i] == 127 || channel[i] == 32 || channel[i] == 44 || channel[i] == 58)
			return (1);
	}
	return 0;
}

int		Server::newChannel(std::string &channel, Client& client){
	if (channel[0] != '#' || channel.size() > 50 ||checkcara(channel))
		return (this->sendMessLocal("476", "", client, "Bad Channel Mask"), 1);
	Channel c = Channel(channel, client);
	this->_channel.push_back(c);
	return (0);
}

int		Server::addClientChannel(std::string &channel, Client& client){
	this->_channel[this->getIndexChannel(channel)].addUser(client);
	return (1);
}




/*-----------------------------------------------------------------------------------------------*/


// supprime le client et son socket de partout
void Server::delClient(int index)
{
	close(this->_clients[index - 1].getFd()); // ferme client socket
	this->_clients.erase(this->_clients.begin() + (index - 1)); // retire le client du gestionnaire
	this->_fds.erase(this->_fds.begin() + index); // retire le client socket des sockets actifs
	return ;
}

void Server::delInvite(void)
{
	for (size_t i = 0; i < this->_channel.size(); i++)
	{
		if (!this->_channel[i].getInvite().empty())
			this->_channel[i].removeInvite();
	}
}

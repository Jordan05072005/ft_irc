/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jguaglio <guaglio.jordan@gmail.com>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/21 13:48:33 by jguaglio          #+#    #+#             */
/*   Updated: 2025/11/22 23:19:53 by jguaglio         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/header.hpp"

Server::Server(void){}

Server& Server::getInstance(void)
{
	static Server instance;
	return instance;
}

static t_cmd init_cmd(std::string n, int (Server::*func)(Client&, std::vector<std::string>&), int state)
{
	t_cmd p;

	p.name = n;
	p.pars = func;
	p.state = state;
	return (p);
}

void	Server::init(int port, std::string const& password)
{
	if (!this->_init)
	{
		this->_init = true;
		this->_port_serv = port;
		this->_close = false;
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
		this->_cmd.push_back(init_cmd("MODE", &Server::checkMode, 2));
		this->_cmd.push_back(init_cmd("PART", &Server::checkPart, 2));
		this->_cmd.push_back(init_cmd("NOTICE", &Server::checkNotice, 2));
		this->_cmd.push_back(init_cmd("LIST", &Server::checkList, 2));
		this->_cmd.push_back(init_cmd("PRIVMSG", &Server::checkPrivmsg, 2));
		this->_cmd.push_back(init_cmd("NAMES", &Server::checkNames, 2));
		this->_cmd.push_back(init_cmd("WHO", &Server::checkWho, 2));

		this->run();
	}

	return ;
}

Server::~Server(void){}

void Server::close_serv(void)
{
	this->_close = true;
	return ;
}

void Server::closeAll(void)
{
	std::string message;
	size_t size = this->_clients.size();
	delete this->_bot[0];
	for (size_t i = 0; i < size; i++)
	{
		std::string message = "ERROR :Closing Link: " + this->_clients[0]->getNick() + " (Server shutting down)\r\n";
		send(this->_clients[0]->getFd(), message.c_str(), message.size(), 0);
		this->delClient(1);
	}
	close(this->_fds[0].fd);
	for (size_t i = 0; i < this->_channel.size(); i++)
		delete this->_channel[i];
	exit(0);
}


/*-----------------------------------------------------------------------------------------------*/


// initialisation de struct pollfd pour chaque socket
static pollfd init_pollfd(int fd, short events, short revents)
{
	pollfd p;

	p.fd = fd; // fd ouvert par le socket
	p.events = events; // évènements à surveiller, POLLIN = true/false if must read in fd
	p.revents = revents; // activation ou non des évènements, le kernell le remplie, 0 par défaut

	return (p);
}

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

	this->_bot.push_back(new Bot("bot42", "bot42", "bot42"));
	while (!this->_close)
	{
		setup_signals();
		err = poll(this->_fds.data(), this->_fds.size(), -1); // poll(pointeur sur tableau, taille tableau, timeout)
		if (err < 0 && !this->_close)
			throw std::runtime_error("Error: poll");
		this->delInvite();

		// traitement server socket
		if (this->_fds[0].revents & POLLIN) // & opération binaire, indique si le flag POLLIN est "activé", renvoie 0 ou POLLIN
		{
			client_fd = accept(this->_fds[0].fd, (sockaddr*)&client_addr, &client_len); // renvoie le socket client et stocke ses données dans une struct
			if (client_fd < 0)
				throw std::runtime_error("Error: acceptance client socket connection");
			fcntl(client_fd, F_SETFL, O_NONBLOCK); // fd/socket non bloquant
			this->_clients.push_back(new Client(client_fd, client_addr, client_len)); // ajout des données client au tableau
			this->_fds.push_back(init_pollfd(client_fd, POLLIN, 0)); // ajout socket client au tableau
			this->_fds[0].revents = 0; // remise à défaut, évènement non actif
			std::cout << "New client, fd : " << client_fd << std::endl;
		}

		// traitement client sockets
		for (size_t i = 1; i < this->_fds.size(); i++)
		{
			if (this->_fds[i].revents & POLLIN)
			{
				buf[0] = '\0';
				oct = recv(this->_fds[i].fd, buf, sizeof(buf), 0); // remplis le buffer en lisant sur le client socket, 0 parce que pas de flag
				if (oct <= 0)
				{
					this->delClient(i--); // envoie i puis enlève 1
					continue; // revient au début de boucle for
				}
				this->_clients[i - 1]->addBuf(buf, oct);
				// this->_clients[i - 1]->getBuf().find("\r\n") != std::string::npos && 
				if (this->_clients[i - 1]->getBuf().find("\n") != std::string::npos && this->requestHandler(*(this->_clients[i - 1])))
				{
					this->delClient(i--);
					continue;
				}
				this->_fds[i].revents = 0; // remise à défaut, évènement non actif
			}
			else if (this->_fds[i].revents & (POLLERR | POLLHUP | POLLNVAL))
				this->delClient(i--);// envoyer un QUIT a tt les autres client,
		}
	}
	this->closeAll();
	return ;
}

// gestion des données reçues
int Server::requestHandler(Client& client)
{
	int err;
	std::vector<std::string> cmd = split2(client.getBuf(), "\r\n");
	if (cmd.empty())
		return (0);
	client.resetBuf();
	for (size_t j = 0; j < cmd.size(); j++){
		std::vector<std::string> mess = split2(cmd[j], " \t");
		err = -1;
		if (mess.size() <= 0)
			continue;
		std::cout << "cmd :" << cmd[j] << std::endl;
		for (size_t i = 0; i < this->_cmd.size(); i++)
		{
			if (mess[0] == this->_cmd[i].name)
			{
				if (this->_cmd[i].state > client.getState()){
					if ((err = this->errorState(client.getState(), mess[0], client)))
						return (1);
				}
				else if ((err = (this->*(_cmd[i].pars))(client, mess)))
					return (1);
			}
		}
		if (err == -1)
			return (this->sendMessLocal("462", mess[0], client, "Unknown command"), 0);
	}
	return 0;
}


/*-----------------------------------------------------------------------------------------------*/


int	Server::checkCap(Client& client, std::vector<std::string>& mess)
{
	std::string message;

	if (mess.size() == 1)
		return 0;
	if (mess[1] == "LS"){
		message = ":irc CAP * LS :\r\n";
		send(client.getFd(), message.c_str(), message.size(), 0);
	}
	else if (mess[1] == "END"){
		if (!client.getIdent().empty() && !client.getNick().empty())
		{
			client.setState(client.getState() + 1);
			this->sendMessLocal("001", "", client, "Welcome to the IRC Network");
		}
	}
	return 0;
}

// vérification du mdp
int Server::checkPass(Client& client, std::vector<std::string>& mess)
{
	std::string err;

	if (mess.size() == 1)
		return (this->sendMessLocal("461", mess[0], client, "Not enough parameters"), 0);
	if (client.getState() >= 1)
		return (this->sendMessLocal("462", "", client, "Unauthorized command (already registered)"), 0);
	if (mess[1] != this->_password)
		return (this->sendMessLocal("464", "", client, "Incorrect password"), 1);

	client.setState(client.getState() + 1);
	return (0);
}

int Server::checkNick(Client& client, std::vector<std::string>& mess)
{
	if (!client.getNick().empty() && client.getState() >= 2)
		return (this->sendMessLocal("462", "", client, "Unauthorized command (already registered)"), 0);
	if (mess.size() == 1)
		return (this->sendMessLocal("431", "", client, "No nickname given"), 0);
	if (this->checkUniqueNick(mess[1]))
		return (this->sendMessLocal("433", "", client, "Nickname is already in use"), 0);
	if (!this->autorisedNick(mess[1]))
		return (this->sendMessLocal("432", mess[1], client, "Erroneous nickname"), 0);
	client.setNick(mess[1]);
	return 0;
}

int Server::checkUser(Client& client, std::vector<std::string>& mess)
{
	std::string realname;

	if ((!client.getRealName().empty() || !client.getIdent().empty()) && client.getState() >= 2)
		return (this->sendMessLocal("462", "", client, "Unauthorized command (already registered)"), 0);
	if (mess.size() < 5) // TODO : reduire a 1 if
		return (this->sendMessLocal("461", "", client, "Not enough parameters"), 0);
	if (mess[4][0] != ':')
		return (this->sendMessLocal("461", "", client, "Not enough parameters"), 0);
	for (int i = 4; i < (int)mess.size(); i++)
	{
		realname += mess[i];
	}
	if (!this->autorisedIdent(mess[1]) || !this->autorisedRealName(realname))
		return (this->sendMessLocal("461", "", client, "Not enough parameters"), 0);
	client.setIdent(mess[1]);
	client.setRealName(realname.erase(0, 1));
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
			for (size_t i = 1; i < mess.size(); i++)
				message += mess[i];
			message.erase(0, 1);
		}
	}
	this->delAllChannelClient(client, mess[0], message);
	if (client.getState() < 2)
		return (1);
	//this->sendMessGlobal(mess[0], message, client);
	return (1);
}

//KICK <channel> <user> [<comment>]
int	Server::checkKick(Client& client, std::vector<std::string>& mess)
{
	std::string message;
	Channel* channel;
	int	i;

	if (mess.size() < 3)
		return (this->sendMessLocal("461", mess[0], client, "Not enough parameters"), 0);
	if (!this->checkExistChannel(mess[1]))
		return (this->sendMessLocal("403", mess[1], client, "No such channel"), 0);
	channel = this->_channel[this->getIndexChannel(mess[1])];
	if (!channel->checkUser(client.getNick()))
		return (this->sendMessLocal("442", mess[1], client, "You're not on that channel"), 0);
	if (!channel->checkOperator(client.getNick()))
		return (this->sendMessLocal("482", mess[1], client, "You're not channel operator"), 0);
	if (!channel->checkUser(mess[2]))
		return (this->sendMessLocal("441", mess[2] + " " + mess[1], client, "They aren't on that channel"), 0);
	if (mess.size() == 3)
		message == "";
	else if (mess[3][0] == ':')
	{
		for (size_t i = 3; i < mess.size(); i++)
			message += (mess[i] + ((i + 1) < mess.size()  ? " " : ""));
		message.erase(0, 1);
	}
	else
		message = mess[3];
	this->sendMessChannel(mess[1], mess[0] + " " + mess[1] + " " + mess[2], message, 1, client);
	channel->removeUser(mess[2]);
	channel->removeOperator(mess[2]);
	client.removeChannel(mess[1]);
	if (channel->getUsers().size() == 0){
		i = this->getIndexChannel(channel->getName());
		delete this->_channel[i];
		this->_channel.erase(this->_channel.begin() + i);
	}
	this->getClient(mess[2]).removeChannel(mess[1]);
	return (0);
}
// :<kicker_nick>!<user>@<host> KICK <channel> <target_nick> :<comment>
// :Alice!~alice@127.0.0.1 KICK #42school johon :Trop de spam
// :server 461 <nick> KICK :Not enough parameters

//INVITE <nickname> <channel>
int Server::checkInvite(Client& client, std::vector<std::string>& mess)
{
	Channel* channel;
	int err = 0;

	if (mess.size() < 3)
		return (this->sendMessLocal("461", mess[0], client, "Not enough parameters"), 0);
	if (!this->checkExistClient(mess[1]) && ++err)
		this->sendMessLocal("401", mess[1], client, "No such nick/channel");
	if (!this->checkExistChannel(mess[2]) && ++err)
		this->sendMessLocal("403", mess[2], client, "No such channel");
	if (err > 0)
			return 0;
	channel = this->_channel[this->getIndexChannel(mess[2])];
	if (!channel->checkUser(client.getNick()))
		return (this->sendMessLocal("442", mess[2], client, "You're not on that channel"), 0);
	if (channel->getOptInviteOnly() && !channel->checkOperator(client.getNick()))
		return (this->sendMessLocal("482", mess[2], client, "You're not channel operator"), 0);
	if (channel->checkUser(mess[1]))
		return (this->sendMessLocal("443", client.getNick() + " " + mess[1] + " " + mess[2], client, "is already on channel"), 0);
	this->sendMessLocal("341", mess[1] + " " + mess[2], client, "");
	this->sendMessLocal("", mess[0], this->getClient(mess[1]), mess[2]);
	channel->addInvite(this->getClient(mess[1]));
	return (0);
}

// TOPIC <Channel> [<newTopic>]
int Server::checkTopic(Client& client, std::vector<std::string>& mess)
{
	Channel* channel;
	std::string message;
	t_topic t;


	if (mess.size() < 2)
		return (this->sendMessLocal("461", mess[0], client, "Not enough parameters"), 0);
	if (!this->checkExistChannel(mess[1]))
		return (this->sendMessLocal("403", mess[1], client, "No such channel"), 0);
	channel = this->_channel[this->getIndexChannel(mess[1])];
	if (!channel->checkUser(client.getNick()))
		return (this->sendMessLocal("442", mess[1], client, "You're not on that channel"), 0);
	if (channel->getOptRestrictTopic()
		&& !channel->checkOperator(client.getNick()))
		return (this->sendMessLocal("482", mess[2], client, "You're not channel operator"), 0);
	if (mess.size() == 2)
	{
		if (channel->getTopic().topic.empty())
			return (this->sendMessLocal("331", mess[1], client, "No topic is set"), 0);
		t = channel->getTopic();
		this->sendMessLocal("332", mess[1], client, t.topic);
		return (this->sendMessLocal("333", mess[1] + " " + t.modifBy + " " + convertTimeStr(t.time), client, ""), 0);
	}
	if (mess[2][0] != ':')
		message = mess[2];
	else
	{
		for (size_t i = 2; i < mess.size(); i++)
			message += (mess[i] + " ");
		message.erase(0, 1);
	}
	channel->setTopic(message, client.getNick());
	return(this->sendMessChannel(mess[1], mess[0] + " " + mess[1], message, 1, client), 0);
}

static int	is_name_ok(std::string const& name)
{
	if (name.size() < 2 || name[0] != '#' || name.size() > 50)
		return (0);
	for (size_t i = 0; i < name.size(); i++)
	{
		if (name[i] == ' ' || name[i] == ',' || name[i] == ':' || name[i] == 7 || name[i] == 13 || name[i] == 10)
			return (0);
	}
	return (1);
}

// JOIN [#channel,&channel] [key,key]
int		Server::checkJoin(Client& client, std::vector<std::string>& mess)
{
	bool						is_invite = false;

	if (mess.size() < 2 || mess.size() > 3)
		return (this->sendMessLocal("461", mess[0], client, "Not enough parameters"), 0);
	if (mess.size() == 2 && mess[1] == "0")
	{
		std::vector<std::string>	cmd;
		cmd.push_back("PART");
		cmd.push_back(this->createStringChannels());
		return (this->checkPart(client, cmd), 0);
	}

	// separation of many commands into one at a time
	std::vector<std::string> mess_cpy = mess;
	if (mess_cpy[1].find(',') != std::string::npos || (mess_cpy.size() == 3 && mess_cpy[2].find(',') != std::string::npos))
	{
		std::vector<std::string> parameters;

		while (mess_cpy[1].find(',') != std::string::npos)
		{
			parameters.push_back(mess_cpy[0]); // "JOIN"
			parameters.push_back(mess_cpy[1].substr(0, mess_cpy[1].find(',')));
			mess_cpy[1] = mess_cpy[1].substr(mess_cpy[1].find(',') + 1);
			if (mess_cpy.size() == 3 && mess_cpy[2].find(',') != std::string::npos)
			{
				if (mess_cpy[2][0] != ',')
					parameters.push_back(mess_cpy[2].substr(0, mess_cpy[2].find(',')));
				mess_cpy[2] = mess_cpy[2].substr(mess_cpy[2].find(',') + 1);
			}
			else if (mess_cpy.size() == 3)
			{
				parameters.push_back(mess_cpy[2]);
				mess_cpy.pop_back();
			}
			this->checkJoin(client, parameters);
			parameters.clear();
		}
		if (mess_cpy.size() == 3 && mess_cpy[2].find(',') != std::string::npos && mess_cpy[2][0] != ',')
			mess_cpy[2] = mess_cpy[2].substr(0, mess_cpy[2].find(','));
	}

	Channel*	channel;
	bool		is_created = false;

	// check channel availability
	if (!this->checkExistChannel(mess_cpy[1]))
	{
		// creating channel at the norm
		if (!is_name_ok(mess_cpy[1]))
			return (this->sendMessLocal("476", mess_cpy[1], client, "Bad Channel Mask"), 0);
		if (mess_cpy.size() == 3)
			this->_channel.push_back(new Channel(ft_tolower(mess_cpy[1]), mess_cpy[2], &client));
		else
			this->_channel.push_back(new Channel(ft_tolower(mess_cpy[1]), &client));
		channel = this->_channel[this->_channel.size() - 1];
		is_created = true;
	}
	else
		channel = this->_channel[this->getIndexChannel(mess_cpy[1])];

	// check if client already on
	if (!is_created && channel->checkUser(client.getNick()))
		return (this->sendMessLocal("443", channel->getName(), client, "is already on channel"),0);

	// check invite-only
	if (!is_created && channel->getOptInviteOnly() == true)
	{
		std::vector<t_invite> const	invite = channel->getInvite();
		for (size_t i = 0; i < invite.size(); ++i)
		{
			if (client.getNick() == invite[i].client->getNick())
			{
				is_invite = true;
				break;
			}
		}

		if (is_invite == false)
			return (this->sendMessLocal("473", mess_cpy[1], client, "Cannot join channel (+i)"), 0);
	}

	// check channel_key
	if (!is_created && channel->getOptChannelKey() == true && (mess_cpy.size() != 3 || (mess_cpy.size() == 3 && channel->getChannelKey() != mess_cpy[2])))
		return (this->sendMessLocal("475", mess_cpy[1], client, "Cannot join channel (+k)"), 0);

	// check user limit
	if (!is_created && channel->getOptUserLimit() == true && channel->getUsersCountNb() >= channel->getUserLimit())
		return (this->sendMessLocal("471", mess_cpy[1], client, "Cannot join channel (+l)"), 0);

	// welcome messages
	if (!is_created)
	{
		client.addChannel(this->_channel[this->getIndexChannel(mess_cpy[1])]);
		channel->addUser(client);
	}
	// :Alice!~alice@host JOIN :#test (par tous users du channel et elle-même)
	if (is_invite)
		channel->delInvite(client);
	this->sendMessChannel(mess_cpy[1], mess_cpy[0], mess_cpy[1], 1, client);
	if (!channel->getTopic().topic.empty())
	{
		this->sendMessLocal("332", mess_cpy[1], client, channel->getTopic().topic);
		this->sendMessLocal("333", mess_cpy[1] + " " + channel->getTopic().modifBy + " " + convertTimeStr(channel->getTopic().time), client, "");
	}
	else
		this->sendMessLocal("331", mess_cpy[1], client, "No topic is set");
	this->sendMessLocal("353", "= " + mess_cpy[1], client, channel->createStringUsers());
	this->sendMessLocal("366", mess_cpy[1], client, "End of /NAMES list");
	return(0);
}

static bool	is_userlimit_ok(std::string const& str)
{
	for (size_t i = 1; i < str.size(); i++)
	{
		if (!std::isdigit(str[i]))
			return (false);
	}
	return (true);
}

// MODE #channel => demande, réponse : :irc.local.net 324 Alice #channel +itk secret
//										:irc.local.net 329 Alice #channel 1730910842

// MODE #channel +itklo secret 10 <client> => demande, réponse toujours sous cette forme
// MODE #channel +itkl-o secret 10 <client> => demande, réponse toujours sous cette forme
// MODE #channel +i +t +k secret +l 10 +o <client> => demande
// MODE #channel +i +t +kl secret 10 +o <client> => demande
// MODE #channel -option <client> => demande

// MODE #channel +i <client> => client ignoré
// MODE #channel +i +t +k secret poulain +l 10 +o <client> cage => poulain et cage ignorés
// MODE #channel +kti secret poulain +l 10 +o <client> cage => poulain et cage ignorés
int		Server::checkMode(Client& client, std::vector<std::string>& mess)
{
	if (mess.size() < 2)
		return (this->sendMessLocal("461", mess[0], client, "Not enough parameters"), 0);
	
	if (!this->checkExistChannel(mess[1]))
		return (this->sendMessLocal("403", mess[1], client, "No such channel"), 0);

	Channel*	channel = this->_channel[this->getIndexChannel(mess[1])];

	if (!channel->checkUser(client.getNick()))
		return (this->sendMessLocal("442", mess[1], client, "You're not on that channel"), 0);

	if (mess.size() > 2)
	{
		if (!channel->checkOperator(client.getNick()))
			return (this->sendMessLocal("482", mess[1], client, "You're not channel operator"), 0);

		std::vector<std::string>	mess_cpy = mess;

		// separation into distinct parameters
		std::vector<std::string>	parameters;
		for (std::vector<std::string>::iterator	it = mess_cpy.begin() + 2; it < mess_cpy.end(); it++)
		{
			if (((*it)[0] == '+' || (*it)[0] == '-') && (*it).size() > 2)
			{
				// separate modestring
				std::vector<std::string>	tmp;
				char						what;
				std::string					mode;
				for (size_t i = 0; i < (*it).size(); i++)
				{
					if ((*it)[i] == '+' || (*it)[i] == '-')
						what = (*it)[i++];
					mode = what + (*it)[i];
					tmp.push_back(mode);
				}
				parameters.insert(parameters.end(), tmp.begin(), tmp.end());
			}
			else
				parameters.push_back(*it);
		}
		
		// sort into pairs of arguments, if a mode needs an argument or not
		typedef std::string mode;
		typedef std::string argument;
		
		std::map<mode, argument>	map;
		for (std::vector<std::string>::iterator itp = parameters.begin(); itp != parameters.end(); itp++)
		{
			if ((*itp)[0] == '+' || (*itp)[0] == '-')
			{
				if ((*itp) == "+k" || (*itp) == "+l" || (*itp) == "+o" || (*itp) == "-o")
				{
					std::vector<std::string>::iterator arg;
					for (arg = itp; arg != parameters.end(); arg++)
					{
						if ((*arg)[0] != '+' && (*arg)[0] != '-')
							break;
					}
					if (arg == parameters.end())
						return (this->sendMessLocal("461", mess[0], client, "Not enough parameters"), 0);
					map.insert(std::pair<mode, argument>(*itp, *arg));
					parameters.erase(arg);
				}
				else
					map.insert(std::pair<mode, argument>(*itp, ""));
			}
		}

		// change modes
		for (std::map<mode, argument>::iterator itm = map.begin(); itm != map.end(); itm++)
		{
			if (itm->first == "+i")
				channel->setOptInviteOnly(true);
			else if (itm->first == "+t")
				channel->setOptRestrictTopic(true);
			else if (itm->first == "+k")
			{
				if (channel->getOptChannelKey() == true && itm->second == channel->getChannelKey())
					return (this->sendMessLocal("467", mess[1], client, "Channel key already set"), 0);
				channel->setOptChannelKey(true);
				channel->setChannelKey(itm->second);
			}
			else if (itm->first == "+l")
			{
				channel->setOptUserLimit(true);
				if (!is_userlimit_ok(itm->second))
					return (this->sendMessLocal("461", mess[0], client, "Not enough parameters"), 0);
				channel->setUserLimit(std::atoi(itm->second.c_str()));
			}
			else if (itm->first == "+o")
			{
				if (!channel->checkUser(itm->second))
					return (this->sendMessLocal("441", itm->second + " " + mess[1], client, "They aren't on that channel"), 0);
				channel->addOperator(this->getClient(itm->second));
			}
			else if (itm->first == "-i")
				channel->setOptInviteOnly(false);
			else if (itm->first == "-t")
				channel->setOptRestrictTopic(false);
			else if (itm->first == "-k")
			{
				channel->setOptChannelKey(false);
				channel->setChannelKey("");
			}
			else if (itm->first == "-l")
			{
				channel->setOptUserLimit(false);
				channel->setUserLimit(0);
			}
			else if (itm->first == "-o")
			{
				if (!channel->checkUser(itm->second))
					return (this->sendMessLocal("441", itm->second + " " + mess[1], client, "They aren't on that channel"), 0);
				channel->removeOperator(itm->second);
			}
			else
				return (this->sendMessLocal("472", itm->first.substr(1, 1), client, "is unknown mode char to me"), 0);
			// TODO : vérifier erreur unknown modestring
		}
		
		// construct response for client
		std::vector<std::string>	order;
		order.push_back("+i");
		order.push_back("+t");
		order.push_back("+k");
		order.push_back("+l");
		order.push_back("+o");

		order.push_back("-i");
		order.push_back("-t");
		order.push_back("-k");
		order.push_back("-l");
		order.push_back("-o");

		parameters.clear();
		parameters.insert(parameters.end(), mess_cpy.begin(), mess_cpy.begin() + 2); // MODE #channel
		for (std::vector<std::string>::iterator ito = order.begin(); ito != order.end(); ito++)
		{
			for (std::map<mode, argument>::iterator itm = map.begin(); itm != map.end(); itm++)
			{
				if (itm->first == *ito)
				{
					if (parameters.size() > 2)
						parameters[2] = add_to_modestring(parameters[2], itm->first);
					else
						parameters.push_back(itm->first);
					if ((*ito) == "+k" || (*ito) == "+l" || (*ito) == "+o" || (*ito) == "-o") // add only necessary args
						parameters.push_back(itm->second);
				}
			}
		}

		std::string	response;
		// we skip MODE #channel
		for (std::vector<std::string>::iterator it = parameters.begin() + 2; it != parameters.end(); it++)
		{
			if (it != parameters.begin() + 2)
				response += ' ';
			response += *it;
		}
		this->sendMessChannel(channel->getName(), parameters[0] + " " + parameters[1] + " " + response, "", 1, client);
	}
	else
	{
		this->sendMessLocal("324", mess[0] + " " + channel->createStringModes(), client, "");
		this->sendMessLocal("329", mess[0] + " " + convertTimeStr(channel->getCreationTime()), client, "");
	}
	std::cout << "ici : " << channel->getOperators().size() << std::endl;
	return (0);
}

// PART #channel,#channel <reason>
// :nick!user@host PART #channel :reason
// :nick!user@host PART #channel
int		Server::checkPart(Client& client, std::vector<std::string>& mess)
{
	int i;
	std::string	reason;

	if (mess.size() < 2)
		return (this->sendMessLocal("461", mess[0], client, "Not enough parameters"), 0);

	// separation of many commands into one at a time
	std::vector<std::string> mess_cpy = mess;
	if (mess_cpy[1].find(',') != std::string::npos)
	{
		std::vector<std::string> parameters;

		while (mess_cpy[1].find(',') != std::string::npos)
		{
			parameters.push_back(mess_cpy[0]); // "PART"
			parameters.push_back(mess_cpy[1].substr(0, mess_cpy[1].find(',')));
			mess_cpy[1] = mess_cpy[1].substr(mess_cpy[1].find(',') + 1);
			for (size_t i = 2; i < mess_cpy.size(); i++)
				parameters.push_back(mess_cpy[i]);
			this->checkPart(client, parameters);
			parameters.clear();
		}
	}
	
	if (!this->checkExistChannel(mess_cpy[1]))
		return (this->sendMessLocal("403", mess_cpy[1], client, "No such channel"), 0);

	Channel*	channel = this->_channel[this->getIndexChannel(mess_cpy[1])];

	if (!channel->checkUser(client.getNick()))
		return (this->sendMessLocal("442", mess_cpy[1], client, "You're not on that channel"), 0);
	if (mess_cpy.size() < 3)
		reason = "";
	else if (mess_cpy[2][0] != ':')
		reason = mess_cpy[2];
	else
	{
		for (size_t i = 2; i < mess_cpy.size(); i++)
		{
			if (i != 2)
				reason += ' ';
			reason += mess_cpy[i];
		}
		reason.erase(0, 1);
	}
	this->sendMessChannel(mess_cpy[1], mess_cpy[0] + " " + mess_cpy[1], reason, 1, client);
	client.removeChannel(mess_cpy[1]);
	channel->removeUser(client.getNick());
	if (channel->getUsers().size() == 0){
		i = this->getIndexChannel(channel->getName());
		delete this->_channel[i];
		this->_channel.erase(this->_channel.begin() + i);
	}
	return (0);
}

// DONT SEND ERRORS JUST DO NOTHING
// NOTICE <msgtarget> <mess>
// if target is channel, all users receives apart himself
int		Server::checkNotice(Client& client, std::vector<std::string>& mess)
{
	if (mess.size() < 3)
		return (1);
	
	std::string message;
	for (size_t i = 2; i < mess.size(); i++)
	{
		if (i != 2)
			message += ' ';
		message += mess[i];
	}
	message.erase(0, 1);

	if (this->checkExistClient(mess[1])) // send mess to someone
	{
		this->sendMessUser(client, this->getClient(mess[1]), mess[0], message);
	}
	else if (this->checkExistChannel(mess[1])) // send mess to channel
	{
		Channel*	channel = this->_channel[this->getIndexChannel(mess[1])];

		if (channel->checkUser(client.getNick())) // if user in channel
			this->sendMessChannel(mess[1], mess[0] + " " + mess[1] , message, 0,client);
	}
	return (0);
}

// LIST
// LIST #channel,#channel
int		Server::checkList(Client& client, std::vector<std::string>& mess)
{
	this->sendMessLocal("321", "Channel", client, "Users Name");

	if (mess.size() > 1)
	{
		// separation of many channels into one at a time
		std::vector<std::string> mess_cpy = mess;
		Channel*	channel;
		while (mess_cpy[1].find(',') != std::string::npos)
		{
			if (this->checkExistChannel(mess_cpy[1].substr(0, mess_cpy[1].find(','))))
			{
				channel = this->_channel[this->getIndexChannel(mess_cpy[1].substr(0, mess_cpy[1].find(',')))];
				if (channel->getTopic().topic.empty())
					this->sendMessLocal("322", channel->getName() + " " + channel->getUsersCountStr(), client, " ");
				else
					this->sendMessLocal("322", channel->getName() + " " + channel->getUsersCountStr(), client, channel->getTopic().topic);
			}
			mess_cpy[1] = mess_cpy[1].substr(mess_cpy[1].find(',') + 1);
		}
	}
	else
	{
		for (std::vector<Channel*>::const_iterator	it = this->_channel.begin(); it != this->_channel.end(); it++)
		{
			if ((*it)->getTopic().topic.empty())
				this->sendMessLocal("322", (*it)->getName() + " " + (*it)->getUsersCountStr(), client, " ");
			else
				this->sendMessLocal("322", (*it)->getName() + " " + (*it)->getUsersCountStr(), client, (*it)->getTopic().topic);
		}
	}

	this->sendMessLocal("323", "", client, "End of /LIST");
	return (0);
}


//PRIVMSG <receiver>{,<receiver>} :<text to be sent>
int		Server::checkPrivmsg(Client& client, std::vector<std::string>& mess)
{
	std::vector<std::string> argm;
	std::string message;

	if (mess.size() < 2)
		return (this->sendMessLocal("461", mess[0], client, "Not enough parameters"), 0);
	if (mess.size() < 3 || mess[2][0] != ':' || mess[2].size() == 1)
		return (this->sendMessLocal("412", "", client, "No text to send"), 0);
	if (client.isMute())
			return (sendMessBot(*this->_bot[0], client, "NOTICE", this->_bot[0]->getMessMute()), 0);
	argm = split(mess[1], ',');
	mess[2].erase(0, 1);
	for (size_t i = 2; i < mess.size(); i++)
		message = message + (mess[i] + ((i + 1) == mess.size() ? "" : " "));
	if (this->_bot[0]->checkMessage(message)){
		client.addWarn();
		return (sendMessBot(*this->_bot[0], client, "NOTICE", this->_bot[0]->getMessBadWords()), 0);
	}
	for (size_t i = 0; i < argm.size(); i++)
	{
		if (argm[i][0] != '#')
		{
			if (!this->checkExistClient(argm[i]))
				this->sendMessLocal("401", argm[i], client, "No such nick/channel");
			else
				this->sendMessUser(client, this->getClient(argm[i]), mess[0], message);
		}
		else
		{
			if (!this->checkExistChannel(argm[i]))
				this->sendMessLocal("403", argm[i], client, "No such channel");
			else if (!this->_channel[this->getIndexChannel(argm[i])]->checkUser(client.getNick()))
				this->sendMessLocal("442", argm[i], client, "You're not on that channel");
			else
				this->sendMessChannel(argm[i], mess[0] + " " + argm[i], message, 0, client);
		}
	}
	return (0);
}

int Server::checkNames(Client& client, std::vector<std::string>& mess)
{
	std::vector<std::string> channels;

	if (mess.size() == 1)
	{
		for (size_t i = 0; i < this->_channel.size(); i++)
		{
			this->sendMessLocal("353", "= " + this->_channel[i]->getName(), client, this->_channel[i]->createStringUsers());
			this->sendMessLocal("366", this->_channel[i]->getName(), client, "End of /NAMES list");
		}
		return (0);
	}
	channels = split(mess[1], ',');
	for (size_t i = 0; i < channels.size(); i++)
	{
		if (!this->checkExistChannel(channels[i]))
			this->sendMessLocal("403", channels[i], client, "No such channel");
		else
		{
			this->sendMessLocal("353", "= " + channels[i], client, this->_channel[this->getIndexChannel(channels[i])]->createStringUsers());
			this->sendMessLocal("366", channels[i], client, "End of /NAMES list");
		}
	}
	return (0);
}

int		Server::checkWho(Client& client, std::vector<std::string>& mess)
{
	std::vector<std::string> channels;
	std::vector<Client*> clients;

	if (mess.size() == 1)
	{
		for (size_t i = 0; i < this->_clients.size(); i++)
		{
			this->sendMessLocal("352", "* " + this->_clients[i]->getIdent() + " " +
				this->_clients[i]->getHost() + " " + 
				this->_clients[i]->getServ() + " " + 
				this->_clients[i]->getNick() + " H", client, "0 " + this->_clients[i]->getRealName());
		}
		this->sendMessLocal("315", "*", client, "End of /WHO list");
		return (0);
	}
	channels = split(mess[1], ',');
	for (size_t i = 0; i < channels.size(); i++)
	{
		if (channels[i][0] == '#' && !this->checkExistChannel(channels[i]))
			this->sendMessLocal("403", channels[i], client, "No such channel");
		else if (channels[i][0] != '#' && !this->checkExistClient(channels[i]))
			this->sendMessLocal("401", channels[i], client, "No such nick");
		else if (channels[i][0] != '#'){
			this->sendMessLocal("352", "* " + this->getClient(channels[i]).getIdent() + " " +
				this->getClient(channels[i]).getHost() + " " + 
				this->getClient(channels[i]).getServ() + " " + 
				this->getClient(channels[i]).getNick() + " H", client, "0 " + 
				this->getClient(channels[i]).getRealName());
			this->sendMessLocal("315", channels[i], client, "End of /WHO list");
		}
		else
		{
			clients = this->_channel[this->getIndexChannel(channels[i])]->getUsers();
			for (size_t j = 0; j < clients.size(); j++)
			{
				this->sendMessLocal("352", channels[i]  + " " + clients[j]->getIdent() + " " +
					clients[j]->getHost() + " " + 
					clients[j]->getServ() + " " + 
					clients[j]->getNick() + " H", client, "0 " + 
					clients[j]->getRealName());
			}
			this->sendMessLocal("315", channels[i], client, "End of /WHO list");
		}
	}
	return (0);
}


/*-----------------------------------------------------------------------------------------------*/


int	Server::checkUniqueNick(std::string const& nick)
{
	for (size_t i = 0; i < this->_clients.size(); i++)
	{
		if (this->_clients[i]->getNick() == nick)
			return 1;
	}
	return 0;
}

int Server::checkExistClient(std::string const& nick)
{
	for (size_t i = 0; i < this->_clients.size(); i++)
	{
		if (this->_clients[i]->getNick() == nick)
			return 1;
	}
	return 0;
}

int	Server::checkExistChannel(std::string const& name)
{
	for (size_t i = 0; i < this->_channel.size(); i++)
	{
		if (this->_channel[i]->getName() == name)
			return (1);
	}
	return (0);
}

int Server::autorisedNick(std::string& name){
	std::string autorised_start = "[]\\_^{ }|`";
	std::string autorised = "-[]\\`_^{}|";
	if (name.size() > 30)
		return (0);
	for (size_t i = 0; i < name.size(); i++){
		if (i == 0 
			&& autorised_start.find(name[i]) == std::string::npos
			&& !(name[i] >= 'A' && name[i] <= 'Z')
			&& !(name[i] >= 'a' && name[i] <= 'z')
		)
			return (0);
		else if (i != 0
			&& autorised.find(name[i]) == std::string::npos
			&& !(name[i] >= 'A' && name[i] <= 'Z')
			&& !(name[i] >= 'a' && name[i] <= 'z')
			&& !(name[i] >= '0' && name[i] <= '9')
		)
			return (0);
	}
	return (1);
}

int Server::autorisedIdent(std::string& name){
	if (name.size() > 30)
		return (0);
	for (size_t i = 0; i < name.size(); i++){
		if (i != 0
			&& name[i] != '_'
			&& !(name[i] >= 'A' && name[i] <= 'Z')
			&& !(name[i] >= 'a' && name[i] <= 'z')
			&& !(name[i] >= '0' && name[i] <= '9')
		)
			return (0);
	}
	return (1);
}

int Server::autorisedRealName(std::string& name){
	if (name.size() > 30)
		return (0);
	for (size_t i = 0; i < name.size(); i++){
		if (i != 0 && name[i] == ':')
			return (0);
	}
	return (1);
}


/*-----------------------------------------------------------------------------------------------*/


int Server::errorState(int state, std::string const& cmd, Client& client)
{
	if (state == 0)
		return (this->sendMessLocal("461", "", client, "Password needed"), 1);
	else if (state  == 1)
		this->sendMessLocal("451", cmd, client, "You have not registered");
	return (0);
}
//:ft_irc 451 JOIN :You have not registered

void Server::sendMessLocal(std::string const& err, std::string const& cmd, Client const& c, std::string const& body)
{
	std::string err_mess;
	std::string nick;
	std::stringstream ss;

	nick = c.getNick();
	if (nick.empty())
		nick = "*";
	if (err.empty())
		ss << ":" << nick << "!" << (c.getIdent().empty() ? "*" : c.getIdent()) << "@" << c.getHost() << (cmd.empty() ? "" : (" " + cmd)) << " " << nick << (body.empty() ? "" : (" :" + body)) << "\r\n";
	else
		ss << ":irc.42.fr " << err << " " << nick << (cmd.empty() ? "" : (" " + cmd)) << (body.empty() ? "" : (" :" + body)) << "\r\n";
	err_mess = ss.str();
	send(c.getFd(), err_mess.c_str(), err_mess.size(), 0);
}

void Server::sendMessBot(Bot& b, Client const& c, std::string cmd, const std::string& mess){
	std::string err_mess;
	std::stringstream ss;

	ss << ":" << b.getNick() << "!"<< b.getIdent() <<"@:irc.42.fr " << cmd << " " << c.getNick() << (mess.empty() ? "" : (" :" + mess)) << "\r\n";
	err_mess = ss.str();
	send(c.getFd(), err_mess.c_str(), err_mess.size(), 0);
}

//:Jo!~jo@127.0.0.1 PRIVMSG Max :Salut Max, ça va ?
void	Server::sendMessUser(Client& s, Client const& r, std::string const& cmd, std::string const& body)
{
	std::string err_mess;
	std::string nick;
	std::stringstream ss;


	nick = s.getNick();
	if (nick.empty())
		nick = "*";
	ss << ":" << nick << "!" << (s.getIdent().empty() ? "*" : s.getIdent()) << "@" << s.getHost() << (cmd.empty() ? "" : (" " + cmd)) << " " << r.getNick() << (body.empty() ? "" : (" :" + body)) << "\r\n";
	err_mess = ss.str();
	send(r.getFd(), err_mess.c_str(), err_mess.size(), 0);
}

void Server::sendMessGlobal(std::string const& cmd, std::string const& mess, Client const& c)
{
	std::stringstream ss;
	std::string message;

	ss << ":" << (c.getNick().empty() ? "*" : c.getNick()) << "!~" << (c.getIdent().empty() ? "*" : c.getIdent()) << "@" << c.getHost() << " " << cmd << (mess.empty() ? "" : (" :" + mess)) << "\r\n";
	message = ss.str();
	for (size_t i = 0; i < this->_clients.size(); i++)
		send(this->_clients[i]->getFd(), message.c_str(), message.size(), 0);
	return ;
}

// TODO: enlever argument
// Alice JOIN :#général
void Server::sendMessChannel(std::string const& channel, std::string const& argm, std::string const& mess, int sendme , Client& c)
{
	std::stringstream ss;
	std::string message;
	int i;
	std::vector<Client*> t;


	ss << ":" << (c.getNick().empty() ? "*" : c.getNick()) << "!~" << (c.getIdent().empty() ? "*" : c.getIdent()) << "@" << c.getHost() << (argm.empty() ? "" : (" " + argm))<< (mess.empty() ? "" : (" :" + mess)) << "\r\n";
	message = ss.str();
	i = this->getIndexChannel(channel);
	t = this->_channel[i]->getUsers();
	for (size_t i = 0; i < t.size(); i++)
	{
		if (sendme || t[i]->getNick() != c.getNick())
			send(t[i]->getFd(), message.c_str(), message.size(), 0);
	}
}

// :john!~john@127.0.0.1 PRIVMSG #general :Salut tout le monde ! 
// pour Channel
// :nick!user@host QUIT :Client Quit


/*-----------------------------------------------------------------------------------------------*/


int	Server::getIndexChannel(std::string const& name)
{
	for (size_t i = 0; i < this->_channel.size(); i++)
	{
		if (this->_channel[i]->getName() == name)
			return (i);
	}
	return (0);
}

Client&	Server::getClient(std::string const& nick)
{
	for (size_t i = 0; i < this->_clients.size(); i++)
	{
		if (this->_clients[i]->getNick() == nick)
			return (*(this->_clients[i]));
	}
	return (*(this->_clients[0])); // TODO : trouver solution pr null
}


/*-----------------------------------------------------------------------------------------------*/


// supprime le client et son socket de partout
void Server::delClient(int index)
{
	Client*	tmp = this->_clients[index - 1];

	close(this->_clients[index - 1]->getFd()); // ferme client socket
	this->_clients.erase(this->_clients.begin() + (index - 1)); // retire le client du gestionnaire
	delete tmp;
	this->_fds.erase(this->_fds.begin() + index); // retire le client socket des sockets actifs
	
	return ;
}

void Server::delInvite(void)
{
	for (size_t i = 0; i < this->_channel.size(); i++)
	{
		if (!this->_channel[i]->getInvite().empty())
			this->_channel[i]->removeInvite();
	}
	return ;
}



//:Alice!alice@host PART #general :Bye everyone!
void	Server::delAllChannelClient(Client& client, std::string& cmd, std::string mess)
{
	std::vector<Channel*> c;
	std::vector<std::string> names;

	c = client.getChannels();
	names.push_back(client.getNick());
	for (size_t i = 0; i < this->_clients.size(); i++)
	{
		for (size_t j = 0; j < c.size(); j++)
		{
			if (c[j]->checkUser(this->_clients[i]->getNick())
				&& std::find(names.begin(), names.end(), this->_clients[i]->getNick()) == names.end())
			{
				this->sendMessUser(client, *this->_clients[i], cmd, mess);
				names.push_back(this->_clients[i]->getNick());
				break;
			}
		}
	}
	std::cout << "mid" << std::endl;
	std::cout << c.size() << std::endl;
	for (size_t i = 0; i < this->_channel.size(); i++)
	{
		this->_channel[i]->removeUser(client.getNick());
		this->_channel[i]->removeOperator(client.getNick());
		// this->sendMessChannel((*it)->getName(), (*it)->getName(), cmd, mess, client);
		if (this->_channel[i]->getUsers().size() == 0)
		{
			delete this->_channel[i];
			this->_channel.erase(this->_channel.begin() + i);
			return ;
		}
	}
	client.removeAllChannels();
	return ;
}


/*-----------------------------------------------------------------------------------------------*/


std::string	Server::createStringChannels(void) const
{
	std::string str;

	for (size_t i = 0; i < this->_channel.size(); i++)
	{
		if (i != 0)
			str += ',';
		str += this->_channel[i]->getName();
	}
	return (str);
}

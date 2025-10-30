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

static t_cmd init_cmd(std::string n, int (Server::*func)(Client&, std::vector<std::string>&), int etat)
{
	t_cmd p;

	p.name = n;
	p.pars = func;
	p.etat = etat;
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

Server::Server(int port, std::string const& password) : _port_serv(port)
{
	this->_password.assign(password);
	this->initServ();
	this->_cmd.push_back(init_cmd("CAP", &Server::checkCap, 0));
	this->_cmd.push_back(init_cmd("PASS", &Server::checkPass, 0));
	this->_cmd.push_back(init_cmd("NICK", &Server::checkNick, 1));
	this->_cmd.push_back(init_cmd("USER", &Server::checkUser, 1));
	this->_cmd.push_back(init_cmd("QUIT", &Server::checkQuit, 1));
	this->_cmd.push_back(init_cmd("KICK", &Server::checkKick, 2));
	// this->_cmd.push_back(init_cmd("QUIT", &Server::checkQuit, 1));
	// this->_cmd.push_back(init_cmd("QUIT", &Server::checkQuit, 1));
	// this->_cmd.push_back(init_cmd("QUIT", &Server::checkQuit, 1));
	// this->_cmd.push_back(init_cmd("QUIT", &Server::checkQuit, 1));
	// this->_cmd.push_back(init_cmd("QUIT", &Server::checkQuit, 1));

	this->run();

	return ;
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
	sockaddr_in	client_addr; // données du client récupérées liées au socket client
	socklen_t		client_len = sizeof(client_addr);
	int 				client_fd; // socket client
	char 				buf[512];
	int 				oct; // nombre d'octets lus renvoyés par recv

	while (1)
	{
		err = poll(this->_fds.data(), this->_fds.size(), -1); // poll(pointeur sur tableau, taille tableau, timeout)
		if (err < 0)
			throw std::runtime_error("Error: poll");
		// traitement server socket
		if (this->_fds[0].revents & POLLIN) // & opération binaire, indique si le flag POLLIN est "activé", renvoie 0 ou POLLIN
		{
			client_fd = accept(this->_fds[0].fd, (sockaddr*)&client_addr, &client_len); // renvoie le socket client et stocke ses données dans une struct
			if (client_fd < 0)
				throw std::runtime_error("Error: acceptance client socket connection");
			fcntl(client_fd, F_SETFL, O_NONBLOCK); // fd/socket non bloquant
			this->_clients.push_back(Client(client_fd, client_addr, client_len)); // ajout des données client au tableau
			std::cout << "after " << this->_clients.size()<< std::endl;
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
		}
	}
	return ;
}

// supprime le client et son socket de partout
void Server::delClient(int index)
{
	close(this->_clients[index - 1].getFd()); // ferme client socket
	this->_clients.erase(this->_clients.begin() + (index - 1)); // retire le client du gestionnaire
	this->_fds.erase(this->_fds.begin() + index); // retire le client socket des sockets actifs
	
	return ;
}

int Server::errorEtat(int etat, std::string& cmd, Client &client)
{
	if (etat == 0)
		return (this->sendMessLocal("461", "",client, "Password needed"), 1);
	else if (etat  == 1)
		this->sendMessLocal("451", cmd, client, "You have not registered");
	return (0);
}
//:ft_irc 451 JOIN :You have not registered

void Server::sendMessLocal(std::string err, std::string cmd, Client& client, std::string body)
{
	std::string err_mess;
	std::string nick;
	std::stringstream ss;

	nick = client.getNick();
	if (nick.empty())
		nick = "*";
	if (cmd.size() != 0)
		ss << ":irc " << err << " " << nick << " " << cmd << " :" << body << "\r\n";
	else
		ss << ":irc " << err << " " << nick << " :" << body << "\r\n";
	err_mess = ss.str();
	send(client.getFd(), err_mess.c_str(), err_mess.size(), 0);
}

void Server::sendMessGlobal(std::string channel, std::string cmd, std::string mess, Client& c)
{
	std::stringstream ss;
	std::string message;

	if (channel.empty())
	{
		ss << ":" << (c.getNick().empty() ? "*" : c.getNick()) << "!~" << (c.getIdent().empty() ? "*" : c.getIdent()) << "@" << c.getHost() << " " << cmd << " :" << mess << "\r\n";
		message = ss.str();
		for (size_t i = 0; i < this->_clients.size(); i++)
			send(this->_clients[i].getFd(), message.c_str(), message.size(), 0);
		return ;
	}
	ss << ":" << (c.getNick().empty() ? "*" : c.getNick()) << "!~" << (c.getIdent().empty() ? "*" : c.getIdent()) << "@" << c.getHost() << " " << cmd << " #" << channel << " :" << mess << "\r\n";
	message = ss.str();
	// for (size_t i = 0; i < this->channel; i++)
	//{
	// 	if (this->channel[i] == channel)
	// 		this->channel[i].sendMessGlobal(message)
	// }
}

// :john!~john@127.0.0.1 PRIVMSG #general :Salut tout le monde ! 
// piur chanel
// :nick!user@host QUIT :Client Quit

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
			if (this->_cmd[i].etat > client.getEtat())
				return (this->errorEtat(client.getEtat(), mess[0], client));
			return (this->*(_cmd[i].pars))(client, mess);
		}
	}
	return (this->sendMessLocal("462", mess[0], client, "Unknown command"), 0);
}

int	Server::uniqueNick(std::string &nick)
{
	for (size_t i = 0; i < this->_clients.size(); i++)
	{
		if (this->_clients[i].getNick() == nick)
			return 1;
	}
	return 0;
}

// vérification du mdp
int Server::checkPass(Client& client, std::vector<std::string>& mess)
{
	std::string err;

	if (client.getEtat() >= 1)
		return (this->sendMessLocal("462", "", client, "Unauthorized command (already registered)"), 0);
	if (mess[1] != this->_password)
		return (this->sendMessLocal("464", "", client, "Incorrect password"), 1);

	client.setEtat(client.getEtat() + 1);

	return (0);
}

int Server::checkNick(Client& client, std::vector<std::string>& mess)
{
	if (!client.getNick().empty())
		return (this->sendMessLocal("462", "", client, "Unauthorized command (already registered)"), 0);
	if (mess.size() == 1)
		return (this->sendMessLocal("431", "", client, "No nickname given"), 0);
	if (this->uniqueNick(mess[1]))
		return (this->sendMessLocal("433", "", client, "Nickname is already in use"), 0);
	client.setNick(mess[1]);
	if (!client.getNick().empty()){
		client.setEtat(client.getEtat() + 1);
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
	if (!client.getNick().empty()){
		client.setEtat(client.getEtat() + 1);
		this->sendMessLocal("001", "", client, "Welcome to the IRC Network");
	}
	return 0;
}

int	Server::checkCap(Client& client, std::vector<std::string>& mess){
	std::string message;
	std::cout << "here" << std::endl;
	message = ":irc CAP * LS :";
	send(client.getFd(), message.c_str(), message.size(), 0);
	(void)mess;
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
	if (client.getEtat() < 2)
		return (1);
	this->sendMessGlobal("", mess[0], message, client);
	return (1);
}

int	Server::checkChannel(std::string &name){
	for (size_t i = 0; i < this->_channel.size(); i++){
		if (this->_channel[i].getName() == name)
			return (1);
	}
	return (0);
}

int	Server::getChannel(std::string &name){
	for (size_t i = 0; i < this->_channel.size(); i++){
		if (this->_channel[i].getName() == name)
			return (i);
	}
	return (0);
}

//KICK <channel> <user> [<comment>]
int	Server::checkKick(Client& client, std::vector<std::string>& mess){
	std::string message;

	if (mess.size() < 4)
		return (this->sendMessLocal("461", mess[0], client, "Not enough parameters"), 0);
	if (!this->checkChannel(mess[1]))
		return (this->sendMessLocal("403", mess[1], client, "No such channel"), 0);
	if (!this->_channel[this->getChannel(mess[1])].checkUser(client.getNick()))
		return (this->sendMessLocal("442", mess[1], client, "You're not on that channel"), 0);
	if (!this->_channel[this->getChannel(mess[1])].checkOperator(client.getNick()))
		return (this->sendMessLocal("482", mess[1], client, "You're not channel operator"), 0);
	if (!this->_channel[this->getChannel(mess[1])].checkUser(mess[2]))
		return (this->sendMessLocal("441", mess[2] + " " + mess[1], client, "They aren't on that channel"), 0);
	this->_channel[this->getChannel(mess[1])].delUsers(mess[2]);
	if (mess[3][1] == ':'){
		for (size_t i = 3; i < mess.size(); i++){
			message += mess[i];
		}
	}
	else
		message = mess[3];
	this->sendMessGlobal(mess[1] + " " + mess[1], mess[0], message, client);
	return (0);
}
// :<kicker_nick>!<user>@<host> KICK <channel> <target_nick> :<comment>
// :Alice!~alice@127.0.0.1 KICK #42school johon :Trop de spam
// :server 461 <nick> KICK :Not enough parameters

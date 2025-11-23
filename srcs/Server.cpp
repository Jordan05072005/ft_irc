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
		this->_cmd.push_back(init_cmd("!help", &Server::checkHelp, 2));
		this->_cmd.push_back(init_cmd("!ping", &Server::checkPing, 2));
		this->_cmd.push_back(init_cmd("!rules", &Server::checkRules, 2));
		this->_cmd.push_back(init_cmd("!stats", &Server::checkStats, 2));
		this->_cmd.push_back(init_cmd("!note", &Server::checkNote, 2));

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
	for (size_t j = 0; j < cmd.size(); j++)
	{
		std::vector<std::string> mess = split2(cmd[j], " \t");
		err = -1;
		if (mess.size() <= 0)
			continue;
		std::cout << "cmd :" << cmd[j] << std::endl;
		for (size_t i = 0; i < this->_cmd.size(); i++)
		{
			if (mess[0] == this->_cmd[i].name)
			{
				if (this->_cmd[i].state > client.getState())
				{
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
	return (0);
}

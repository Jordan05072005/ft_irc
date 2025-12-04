#include "../includes/header.hpp"

Server::Server(void){}

Server::Server(const Server& other){
	*this = other;
}

Server& Server::operator=(const Server& other){
	if (this != &other){
		this->_bot = other._bot;
		this->_port_serv = other._port_serv;
		this->_password = other._password;
		this->_init = other._init;
		this->_fds = other._fds;
		this->_cmd = other._cmd;
		this->_close = other._close;
		this->_clients = other._clients;
		this->_channel = other._channel;
	}
	return (*this);
}

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
		this->_cmd.push_back(init_cmd("whois", &Server::checkWhois, 2));
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


// intit struct pollfd for each socket
static pollfd init_pollfd(int fd, short events, short revents)
{
	pollfd p;

	p.fd = fd; // socket
	p.events = events; // event to look at, POLLIN = true/false if must read in fd
	p.revents = revents; // event activation true/false, filled by kernell, 0 is default

	return (p);
}

// ceating base socket to receive client's connections
void Server::initServ(void)
{
	int opt = 1; //? pk 1

	this->_fds.push_back(init_pollfd(socket(AF_INET, SOCK_STREAM, 0), POLLIN, 0)); // socket(ipv4, SOCK_STREAM <=> must listen and accept, TCP is default)
	if (this->_fds[0].fd == -1)
		throw std::runtime_error("Error: server socket creation");

	// setting connection's options
	this->_addr.sin_family = AF_INET; // ipv4
	this->_addr.sin_port = htons(this->_port_serv); // convert port for struct addr
	this->_addr.sin_addr.s_addr = INADDR_ANY; // connection free for every machine's interfaces hosting the server
	fcntl(this->_fds[0].fd, F_SETFL, O_NONBLOCK); // fd/socket non blocking

	// reuse free of port after server's crash for every interfaces used
	setsockopt(this->_fds[0].fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	this->bindAndListen(this->_addr);

	return ;
}

// binding of connection's options to socket
// then putting socket on listen
void Server::bindAndListen(sockaddr_in const& addr)
{
	if (bind(this->_fds[0].fd, (sockaddr*)&addr, sizeof(addr)) < 0)
		throw std::runtime_error("Error: binding server socket");
	if (listen(this->_fds[0].fd, SOMAXCONN) < 0) // SOMAXCONN = size max of waiting list
		throw std::runtime_error("Error: listening on server socket");

	return ;
}

// running server
void Server::run(void)
{
	int					err;

	sockaddr_in			client_addr; // client's connection's options
	socklen_t			client_len = sizeof(client_addr);
	int 				client_fd; // client socket

	char 				buf[512];
	int 				oct; // nb of octet read by recv

	this->_bot.push_back(new Bot("bot42", "bot42", "bot42"));
	while (!this->_close)
	{
		setup_signals();
		err = poll(this->_fds.data(), this->_fds.size(), -1); // poll(pointer to array, size of array, timeout)
		if (err < 0 && !this->_close)
			throw std::runtime_error("Error: poll");
		this->delInvite();

		// managing entering connections
		if (this->_fds[0].revents & POLLIN) // & is binary operator, 0 or POLLIN
		{
			client_fd = accept(this->_fds[0].fd, (sockaddr*)&client_addr, &client_len); // give new socket for new client and fill connection's options of client socket in struct
			if (client_fd < 0)
				throw std::runtime_error("Error: acceptance client socket connection");
			if (fcntl(client_fd, F_SETFL, O_NONBLOCK) < 0)
				std::perror("fcntl(O_NONBLOCK)");
			this->_clients.push_back(new Client(client_fd, client_addr, client_len)); // add of client's socket's data to array
			this->_fds.push_back(init_pollfd(client_fd, POLLIN, 0)); // add client's open socket to array of sockets to manage
			this->_fds[0].revents = 0; // put to default
			std::cout << "New client, fd : " << client_fd << std::endl;
		}

		// managing client's received data
		for (size_t i = 1; i < this->_fds.size(); i++)
		{
			if (this->_fds[i].revents & POLLIN)
			{
				std::memset(buf,0, sizeof(buf));
				oct = recv(this->_fds[i].fd, buf, sizeof(buf), 0); // 0 is no flag
				if (oct <= 0)
				{
					this->delClient(i--);
					continue; // beginning of for
				}
				this->_clients[i - 1]->addBuf(buf, oct);
				if (this->_clients[i - 1]->getBuf().find("\r\n") != std::string::npos && this->requestHandler(*(this->_clients[i - 1])))
				{
					this->delClient(i--);
					continue;
				}
				this->_fds[i].revents = 0;
			}
			else if (this->_fds[i].revents & (POLLERR | POLLHUP | POLLNVAL)) // ? à remplir
				this->delClient(i--);
		}
	}
	this->closeAll();
	return ;
}

// received data handler
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
				client.setLastActivity();
			}
		}
		if (err == -1)
			return (this->sendMessLocal("462", mess[0], client, "Unknown command"), 0);
	}
	return (0);
}

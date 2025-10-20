#include "includes/ft_irc.hpp"

ft_irc::ft_irc(){}

ft_irc::ft_irc(int port) : _port_serv(port){}

void ft_irc::bindAndListen(const sockaddr_in &addr){
	if (bind(this->_fds[0].fd, (sockaddr*)&addr, sizeof(addr)) < 0)
		throw std::runtime_error("Erreur lors du bind");
	if (listen(this->_fds[0].fd, SOMAXCONN) < 0)
		throw std::runtime_error("Erreur lors du listen");

}

static pollfd init_pollfd(int fd, short events, short revents){
	pollfd p;

	p.fd = fd;
	p.events = events;
	p.revents = revents;
	return (p);
}


void ft_irc::initSev(){
	int opt = 1;

	this->_fds.push_back(init_pollfd(socket(AF_INET, SOCK_STREAM, 0), POLLIN, 0)); // ipv4 et tcp
	if (this->_fds[0].fd == -1)
		throw std::runtime_error("Erreur lors du socket");
	this->_addr.sin_family = AF_INET; //configg pour ipv4
	this->_addr.sin_port = htons(this->_port_serv);
	this->_addr.sin_addr.s_addr = INADDR_ANY; // accpete tout les ports de connexion
	fcntl(this->_fds[0].fd, F_SETFL, O_NONBLOCK);
	setsockopt(this->_fds[0].fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	this->bindAndListen(this->_addr);
}

void ft_irc::startSev(){
	int					err;
	sockaddr_in	client_addr;
	socklen_t client_len = sizeof(client_addr);
	int client_fd;
	
	std::cout << "mia" << std::endl;

	while (1){
		err = poll(this->_fds.data(), this->_fds.size(), -1);
		if (err < 0)
			throw std::runtime_error("Erreur lors du pool");
		if (this->_fds[0].revents & POLLIN){
			client_fd = accept(this->_fds[0].fd, (sockaddr*)&client_addr, &client_len);
			if (client_fd < 0)
				throw std::runtime_error("Erreur lors du accept");
			// this->clients.push_back(client(client_fd, client_addr, client_len));
			this->_fds.push_back(init_pollfd(client_fd, POLLIN, 0));
			std::cout << "hello" << std::endl;
		}
		for (std::vector<pollfd>::size_type i = 1; i < this->_fds.size(); i++){
			if (this->_fds[i].revents & POLLIN){
				return ;
			}
		}
		
	}
}
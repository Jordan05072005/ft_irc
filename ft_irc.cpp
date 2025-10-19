#include "includes/ft_irc.hpp"

ft_irc::ft_irc(){}

ft_irc::ft_irc(int port) : _port_serv(port){}

void ft_irc::bindAndListen(const sockaddr_in &addr){
	if (bind(this->_serv_fd, (sockaddr*)&addr, sizeof(addr)) < 0)
		throw std::runtime_error("Erreur lors du bind");
	if (listen(this->_serv_fd, SOMAXCONN) < 0)
		throw std::runtime_error("Erreur lors du listen");

}

void ft_irc::initSev(){
	int opt = 1;

	this->_serv_fd = socket(AF_INET, SOCK_STREAM, 0); // ipv4 et tcp
	if (this->_serv_fd == -1)
		throw std::runtime_error("Erreur lors du socket");
	this->addr.sin_family = AF_INET; //configg pour ipv4
	this->addr.sin_port = htons(this->_port_serv);
	this->addr.sin_addr.s_addr = INADDR_ANY; // accpete tout les ports de connexion
	fcntl(this->_serv_fd, F_SETFL, O_NONBLOCK);
	setsockopt(this->_serv_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	this->bindAndListen(this->addr);
}
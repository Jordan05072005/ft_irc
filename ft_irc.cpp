/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_irc.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jguaglio <guaglio.jordan@gmail.com>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/21 13:48:33 by jguaglio          #+#    #+#             */
/*   Updated: 2025/10/21 13:48:33 by jguaglio         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "includes/ft_irc.hpp"
#include "includes/utils.hpp"

ft_irc::ft_irc(){}

ft_irc::ft_irc(int port, char *password) : _port_serv(port){
	this->_password.assign(password);
	this->initSev();
}

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

void ft_irc::delClient(int i){
	close(this->_clients[i - 1].getfd());
	this->_clients.erase(this->_clients.begin() + (i - 1));
	this->_fds.erase(this->_fds.begin() + i);
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
	int 	client_fd;
	char buf[512];
	int oct;

	while (1){
		err = poll(this->_fds.data(), this->_fds.size(), -1);
		if (err < 0)
			throw std::runtime_error("Erreur lors du pool");
		if (this->_fds[0].revents & POLLIN){
			client_fd = accept(this->_fds[0].fd, (sockaddr*)&client_addr, &client_len);
			if (client_fd < 0)
				throw std::runtime_error("Erreur lors du accept");
			fcntl(client_fd, F_SETFL, O_NONBLOCK);
			this->_clients.push_back(client(client_fd, client_addr, client_len));
			this->_fds.push_back(init_pollfd(client_fd, POLLIN, 0));
			this->_fds[0].revents = 0;
		}
		for (int i = 1; i < (int)this->_fds.size(); i++){
			if (this->_fds[i].revents & POLLIN){
				oct = recv(this->_fds[i].fd, buf, sizeof(buf), 0);
				if (oct <= 0){
					this->delClient(i--);
					continue;
				}
				this->_clients[i - 1].setbuf(buf, oct);
				std::cout << this->_clients[i - 1].getbuf() << std::endl;
				this->requeteGestion(this->_clients[i - 1], &i);
				// send(this->_fds[i].fd, buf, oct, 0);
				this->_fds[i].revents = 0;
			}
		}
	}
}

int ft_irc::checkPass(client& client, int *i, std::vector<std::string> mess){
	std::string err;
	if (mess[0] == "PASS"){
		mess[1].erase(mess[1].find_last_not_of("\r\n") + 1);
		if (mess[1] != this->_password){
			err = ":irc 464 " +  client.getnick() + " :Password incorrect\r\n";
			send(client.getfd(), err.c_str(), err.size(), 0);
			return (this->delClient((*i)--), 1);
		}
		client.setlogin(true);
	}
	else{
		err = ":irc 461 " + client.getnick() + " :Need password\r\n";
		send(client.getfd(), err.c_str(), err.size(), 0);
		return (this->delClient((*i)--), 1);
	}
	return 0;
}

void ft_irc::requeteGestion(client& client, int *i){
	std::vector<std::string> mess = split(client.getbuf(), ' ');
	if (!client.getlogin())
		if (checkPass(client, i, mess))
			return;
	//if...
}

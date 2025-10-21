/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jguaglio <guaglio.jordan@gmail.com>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/21 13:48:31 by jguaglio          #+#    #+#             */
/*   Updated: 2025/10/21 13:48:31 by jguaglio         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "includes/client.hpp"


Client::Client(){}

Client::Client(int fd, sockaddr_in addr, socklen_t len) : _fd(fd), _addr(addr), _len(len), _nick("*"), _login(false){}

Client::Client(const Client& cpy){
	*this = cpy;
}

Client& Client::operator=(const Client& cpy){
	if (this != &cpy){
		this->_fd = cpy._fd;
		this->_buff = cpy._buff;
		this->_nick = cpy._nick;
	}
	// cpy.setfd(-1);
	return (*this);
}

Client::~Client(){}

int Client::getfd(){
	return (this->_fd);
}

void Client::setfd(int fd){
	this->_fd = fd;
}

std::string& Client::getbuf(){
	return (this->_buff);
}

void Client::setbuf(char *buf, int oct){
	if (oct > 0)
		this->_buff.assign(buf, oct); 
}

std::string& Client::getnick(){
	return (this->_nick);
}

void Client::setnick(std::string& nick){
	this->_nick = nick; 
}

bool Client::getlogin(){
	return (this->_login);
}

void Client::setlogin(bool e){
	this->_login = e; 
}






/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jguaglio <guaglio.jordan@gmail.com>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/21 13:48:31 by jguaglio          #+#    #+#             */
/*   Updated: 2025/10/21 13:48:31 by jguaglio         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "includes/client.hpp"

client::client(){}

client::client(int fd, sockaddr_in addr, socklen_t len) : _fd(fd), _addr(addr), _len(len){}

client::client(const client& cpy){
	*this = cpy;
}

client& client::operator=(const client& cpy){
	if (this != &cpy){
		this->_fd = cpy._fd;
		this->_buff = cpy._buff;
		this->_name = cpy._name;
	}
	// cpy.setfd(-1);
	return (*this);
}

client::~client(){}

int client::getfd(){
	return (this->_fd);
}

void client::setfd(int fd){
	this->_fd = fd;
}

std::string& client::getbuf(){
	return (this->_buff);
}

void client::setbuf(char *buf, int oct){
	if (oct > 0)
		this->_buff.assign(buf, oct); 
}






#include "includes/header.hpp"


Client::Client(void){}

Client::Client(int fd, sockaddr_in addr, socklen_t len) : _fd(fd), _addr(addr), _len(len), _nick("*"), _login(false){}

Client::Client(const Client& cpy)
{
	*this = cpy;
}

Client& Client::operator=(const Client& cpy)
{
	if (this != &cpy)
	{
		this->_fd = cpy._fd;
		this->_buff = cpy._buff;
		this->_nick = cpy._nick;
	}
	// cpy.setFd(-1);
	return (*this);
}

Client::~Client(void){}

int Client::getFd(void)
{
	return (this->_fd);
}

void Client::setFd(int fd)
{
	this->_fd = fd;
}

std::string& Client::getBuf(void)
{
	return (this->_buff);
}

void Client::setBuf(char *buf, int oct)
{
	if (oct > 0)
		this->_buff.assign(buf, oct); 
}

std::string& Client::getNick(void)
{
	return (this->_nick);
}

void Client::setNick(std::string& nick)
{
	this->_nick = nick; 
}

bool Client::getLogin(void)
{
	return (this->_login);
}

void Client::setLogin(bool e)
{
	this->_login = e; 
}

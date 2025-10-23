#include "includes/header.hpp"


Client::Client(void){}

Client::Client(int fd, sockaddr_in addr, socklen_t len) : _fd(fd), _addr(addr), _len(len), _etat(0)
{
	char ip_str[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &(addr.sin_addr), ip_str, INET_ADDRSTRLEN);
	this->_host = ip_str;
}

Client::Client(const Client& cpy) : _etat(0)
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
		this->_addr = cpy._addr;
		this->_len = cpy._len;
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

bool Client::getEtat(void)
{
	return (this->_etat);
}

void Client::setEtat(int e)
{
	this->_etat = e;
}

std::string& Client::getIdent(void)
{
	return (this->_ident);
}

void Client::setIdent(std::string& ident)
{
	this->_ident = ident;
}


std::string& Client::getRealName(void)
{
	return (this->_realname);
}

void Client::setRealName(std::string& name)
{
	this->_realname= name;
}


std::string& Client::getHost(void)
{
	return (this->_host);
}

void Client::setHost(std::string& host)
{
	this->_host = host;
}

//nick!~ident@host
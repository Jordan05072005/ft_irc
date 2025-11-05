#include "includes/header.hpp"


Client::Client(void){}

Client::Client(int fd, sockaddr_in addr, socklen_t len) : _fd(fd), _addr(addr), _len(len), _state(1)
{
	char ip_str[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &(addr.sin_addr), ip_str, INET_ADDRSTRLEN);
	this->_host = ip_str;
}

Client::Client(const Client& cpy) : _state(1)
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
		this->_state = cpy._state;
		this->_channel = cpy._channel;
		this->_realname = cpy._realname;
		this->_ident = cpy._ident;
	}
	// cpy.setFd(-1);
	return (*this);
}

Client::~Client(void){}


/*-----------------------------------------------------------------------------------------------*/


int Client::getFd(void) const
{
	return (this->_fd);
}

void Client::setFd(int fd)
{
	this->_fd = fd;
}


/*-----------------------------------------------------------------------------------------------*/


std::string const& Client::getBuf(void) const
{
	return (this->_buff);
}

void Client::setBuf(char *buf, int oct)
{
	if (oct > 0)
		this->_buff.assign(buf, oct); 
}


/*-----------------------------------------------------------------------------------------------*/


const std::string& Client::getNick(void) const
{
	return (this->_nick);
}

void Client::setNick(std::string& nick)
{
	this->_nick = nick; 
}

const std::string& Client::getIdent(void) const
{
	return (this->_ident);
}

void Client::setIdent(std::string& ident)
{
	this->_ident = ident;
}


const std::string& Client::getRealName(void) const
{
	return (this->_realname);
}

void Client::setRealName(std::string& name)
{
	this->_realname= name;
}


const std::string& Client::getHost(void) const
{
	return (this->_host);
}

void Client::setHost(std::string& host)
{
	this->_host = host;
}

//nick!~ident@host

/*-----------------------------------------------------------------------------------------------*/


void	Client::addChannel(Channel& name)
{
	this->_channel.push_back(name);
	return ;
}

void	Client::removeAllChannels(void)
{
	for (size_t i = 0; i < this->_channel.size(); ++i)
	{
		this->_channel[i].removeUser(this->_nick);
		// TODO : add message de sortie de channel
	}
	return ;
}


/*-----------------------------------------------------------------------------------------------*/


int Client::getState(void) const
{
	return (this->_state);
}

void Client::setState(int e)
{
	this->_state = e;
}

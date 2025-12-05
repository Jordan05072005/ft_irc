#include "../includes/header.hpp"


Client::Client(void){}

Client::Client(int fd, sockaddr_in addr, socklen_t len) : _fd(fd), _addr(addr), _len(len), _state(0)
{
	char ip_str[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &(addr.sin_addr), ip_str, INET_ADDRSTRLEN);
	this->_host = ip_str;
	this->_serv = "irc.42.fr";
	this->_buff = "";
	this->_last_activity = std::time(NULL);
	this->_mute.warn = 0;
	this->_mute.mute = false;
}

Client::Client(const Client& cpy)
{
	*this = cpy;
}

Client& Client::operator=(const Client& cpy)
{
	if (this != &cpy)
	{
		this->_fd = cpy._fd;
		this->_addr = cpy._addr;
		this->_len = cpy._len;
		this->_state = cpy._state;

		this->_buff = cpy._buff;
		this->_last_activity = cpy._last_activity;

		this->_nick = cpy._nick;
		this->_ident = cpy._ident;
		this->_realname = cpy._realname;
		this->_host = cpy._host;
		this->_serv = cpy._serv;
		this->_message = cpy._message;

		this->_channel = cpy._channel;
		this->_mute = cpy._mute;
	}
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

void Client::addBuf(char *buf, int len)
{
	for (int i = 0; i < len; i++)
	{
		if (buf[i] >= 32 || buf[i] == 13 || buf[i] == 10 || buf[i] == 0) // printable || carriage return || new line || NUL
			this->_buff += buf[i]; 
	}
	return ;
}

void Client::resetBuf(void)
{
	this->_buff = "";
}


/*-----------------------------------------------------------------------------------------------*/


std::string const& Client::getNick(void) const
{
	return (this->_nick);
}

void Client::setNick(std::string& nick)
{
	this->_nick = nick; 
}

std::string const& Client::getIdent(void) const
{
	return (this->_ident);
}

void Client::setIdent(std::string& ident)
{
	this->_ident = ident;
}


std::string const& Client::getRealName(void) const
{
	return (this->_realname);
}

void Client::setRealName(std::string& name)
{
	this->_realname = name;
}


std::string const& Client::getHost(void) const
{
	return (this->_host);
}

void Client::setHost(std::string& host)
{
	this->_host = host;
}

//nick!~ident@host

/*-----------------------------------------------------------------------------------------------*/


std::vector<Channel*>&	Client::getChannels(void)
{
	return (this->_channel);
}

std::string	Client::getChannelsList(void)
{
	std::string str;

	for (size_t i = 0; i < this->_channel.size(); ++i)
	{
		if (i > 0)
			str += " ";
		str += this->_channel[i]->getName();
	}
	if (str.empty())
		str += " ";
	return (str);
}

void	Client::addChannel(Channel* channel)
{
	this->_channel.push_back(channel);
	return ;
}

void	Client::removeChannel(std::string const& name)
{
	std::string namelower = ft_tolower(name);
	for (size_t i = 0; i < this->_channel.size(); i++)
	{
		if (ft_tolower(this->_channel[i]->getName()) == namelower)
			this->_channel.erase(this->_channel.begin() + i);
	}
	return ;
}

void	Client::removeAllChannels(void)
{
	this->_channel.clear();
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

std::string const&	Client::getServ(void) const
{
	return (this->_serv);
}

void	Client::setServ(std::string& host)
{
	this->_serv = host;
}

int	Client::getWarn(void) const
{
	return (this->_mute.warn);
}

void	Client::addWarn(void)
{
	this->_mute.warn++;
	if (this->_mute.warn >= 3)
	{
		this->_mute.mute = true;
		this->_mute.end = std::time(NULL) + 5 * 60;
	}
}

void	Client::resetWarn(void)
{
	this->_mute.warn = 0;
}

bool	Client::isMute(void)
{
	if (this->_mute.mute)
	{
		std::time_t now = std::time(NULL);
		if (now < this->_mute.end)
			return (1);
		this->_mute.mute = false;
		this->resetWarn();
	}
	return (0);
}

time_t	Client::getIdle(void) const
{
	time_t now = std::time(NULL);
	return (now - this->_last_activity);
}

void	Client::setLastActivity(void)
{
	time_t now = std::time(NULL);
	this->_last_activity = now;
}

std::string const&		Client::getMess(void) const
{
	return (this->_message);
}

void	Client::setMess(std::string& mess)
{
	this->_message = mess;
}

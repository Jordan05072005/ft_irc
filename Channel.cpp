#include "includes/header.hpp"

Channel::Channel(void){}

Channel::Channel(Channel const& copy)
{
	*this = copy;
	return ;
}

Channel& Channel::operator=(Channel const& other)
{
	if (this != &other)
	{
		this->_name = other._name;
		this->_topic = other._topic;
		this->_users = other._users;
		this->_operators = other._operators;
		this->_invite = other._invite;
		this->_channel_key = other._channel_key;
		this->_user_limit = other._user_limit;

		this->_i = other._i;
		this->_t = other._t;
		this->_k = other._k;
		this->_l = other._l;
	}
	return (*this);
}

Channel::Channel(std::string const& _name, Client* creator)
{
	this->_creationtime = std::time(NULL);
	this->_name = _name;
	this->_topic.topic = "";
	this->_topic.modifBy = "";
	this->_topic.time = 0;
	this->_users.push_back(creator);
	this->_operators.push_back(creator);

	creator->addChannel(this);

	this->_channel_key = "";
	this->_user_limit = 0; // no limit

	this->_i = false;
	this->_t = false;
	this->_k = false;
	this->_l = false;
	return ;
}

Channel::Channel(std::string const& _name, std::string const& key, Client* creator)
{
	this->_name = _name;
	this->_topic.topic = "";
	this->_topic.modifBy = "";
	this->_topic.time = 0;
	this->_users.push_back(creator);
	this->_operators.push_back(creator);

	creator->addChannel(this);

	this->_channel_key = key;
	this->_user_limit = 0; // no limit

	this->_i = false;
	this->_t = false;
	this->_k = true;
	this->_l = false;
	return ;
}

Channel::~Channel(void){}


/*-----------------------------------------------------------------------------------------------*/


std::time_t const&	Channel::getCreationTime(void) const
{
	return (this->_creationtime);
}

std::string const&	Channel::getName(void) const
{
	return (this->_name);
}

const t_topic& Channel::getTopic(void) const
{
	return (this->_topic);
}

void 	Channel::setTopic(const std::string& topic, const std::string& nick)
{
	std::time_t now = std::time(NULL);
	
	this->_topic.topic = topic;
	this->_topic.time = now;
	this->_topic.modifBy = nick;
}

std::string const&	Channel::getChannelKey(void) const
{
	return (this->_channel_key);
}

void	Channel::setChannelKey(std::string const& channel_key)
{
	this->_channel_key = channel_key;
	return ;
}

size_t	Channel::getUserLimit(void) const
{
	return (this->_user_limit);
}

void	Channel::setUserLimit(size_t nb)
{
	this->_user_limit = nb;
	return ;
}

bool Channel::getOptInviteOnly(void) const
{
	return (this->_i);
}

void	Channel::setOptInviteOnly(bool opt)
{
	this->_i = opt;
	return ;
}

bool Channel::getOptRestrictTopic(void) const
{
	return (this->_t);
}

void	Channel::setOptRestrictTopic(bool opt)
{
	this->_t = opt;
	return ;
}

bool	Channel::getOptChannelKey(void) const
{
	return (this->_k);
}

void	Channel::setOptChannelKey(bool opt)
{
	this->_k = opt;
	return ;
}

bool	Channel::getOptUserLimit(void) const
{
	return (this->_l);
}

void	Channel::setOptUserLimit(bool opt)
{
	this->_l = opt;
	return ;
}

/*-----------------------------------------------------------------------------------------------*/


std::vector<Client*> const&		Channel::getUsers(void) const
{
	return (this->_users);
}

std::string						Channel::getUsersCountStr(void) const
{
	std::stringstream	ss;

	ss << this->_users.size();
	return (ss.str());
}

size_t							Channel::getUsersCountNb(void) const
{
	return (this->_users.size());
}

std::vector<Client*> const&		Channel::getOperators(void) const
{
	return (this->_operators);
}

const std::vector<t_invite>&	Channel::getInvite(void) const
{
	return (this->_invite);
}


/*-----------------------------------------------------------------------------------------------*/


void	Channel::addUser(Client& user)
{
	this->_users.push_back(&user);
	return ;
}

void	Channel::addOperator(Client& user)
{
	this->_operators.push_back(&user);
	return ;
}

void Channel::addInvite(Client& client)
{
	t_invite invite;

	invite.client = &client;
	invite.time = std::time(NULL);
	if (this->checkInvite(client.getNick()))
		return ;
	this->_invite.push_back(invite);
}


/*-----------------------------------------------------------------------------------------------*/


void	Channel::removeUser(std::string const& nick)
{
	for (size_t i = 0; i < this->_users.size(); i++)
	{
		if (this->_users[i]->getNick() == nick)
			this->_users.erase(this->_users.begin() + i);
	}
	this->removeOperator(nick);
	return ;
}

void	Channel::removeOperator(std::string const& nick)
{
	for (size_t i = 0; i < this->_operators.size(); i++)
	{
		if (this->_operators[i]->getNick() == nick){
			this->_operators.erase(this->_operators.begin() + i);
		}
	}
	return ;
}

void	Channel::removeInvite(void)
{
	std::time_t now = std::time(NULL);
	for (size_t i = 0; i < this->_invite.size(); i++)
	{
		if (now - this->_invite[i].time >= 600)
			this->_invite.erase(this->_invite.begin() + i);
	}
}

void Channel::delInvite(Client &c){
	for (size_t i = 0; i < this->_invite.size(); i++){
		if (this->_invite[i].client->getNick() == c.getNick()){
			this->_invite.erase(this->_invite.begin() + i);
		}
	}
}


/*-----------------------------------------------------------------------------------------------*/


int Channel::checkUser(const std::string& nick) const
{
	for (size_t i = 0; i < this->_users.size(); i++)
	{
		if (this->_users[i]->getNick() == nick)
			return (1);
	}
	return (0);
}

int Channel::checkOperator(const std::string& nick) const
{
	for (size_t i = 0; i < this->_operators.size(); i++)
	{
		if (this->_operators[i]->getNick() == nick)
			return (1);
	}
	return (0);
}

int Channel::checkInvite(const std::string& nick) const
{
	for (size_t i = 0; i < this->_invite.size(); i++)
	{
		if (this->_invite[i].client->getNick() == nick)
			return (1);
	}
	return (0);
}


/*-----------------------------------------------------------------------------------------------*/


std::string	Channel::createStringUsers(void) const
{
	std::string str;

	for (size_t i = 0; i < this->_users.size(); ++i)
	{
		if (i > 0)
			str += " ";
		if (this->checkOperator(this->_users[i]->getNick()))
			str += "@";
		str += this->_users[i]->getNick();
	}
	return (str);
}

std::string	Channel::createStringModes(void) const
{
	std::vector<std::string>	modes;
	std::string	str = "";

	modes.push_back(str);
	if (this->_i == true || this->_t == true || this->_k == true || this->_l == true)
	{
		modes[0] += '+';
	}
	else
		return (str);
	
	if (this->_i == true)
		modes[0] = add_to_modestring(modes[0], "+i");
	if (this->_t == true)
		modes[0] = add_to_modestring(modes[0], "+t");
	if (this->_k == true)
	{
		modes.push_back(this->_channel_key);
		modes[0] = add_to_modestring(modes[0], "+k");
	}
	if (this->_l == true)
	{
		std::string tmp;
		std::sprintf(&tmp[0], "%ld", this->_user_limit);
		modes.push_back(tmp);
		modes[0] = add_to_modestring(modes[0], "+l");
	}

	for (std::vector<std::string>::iterator it = modes.begin(); it != modes.end(); ++it)
	{
		if (it != modes.begin())
			str += ' ';
		str += *it;
	}
	return (str);
}

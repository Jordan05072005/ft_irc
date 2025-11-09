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


std::string const&	Channel::getName(void) const
{
	return (this->_name);
}

const t_topic& Channel::getTopic(void) const
{
	return (this->_topic);
}

void 	Channel::setTopic(const std::string& topic, const std::string &nick)
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

bool Channel::getOptInviteOnly(void) const
{
	return (this->_i);
}

bool Channel::getOptRestrictTopic(void) const
{
	return (this->_t);
}

bool	Channel::getOptChannelKey(void) const
{
	return (this->_k);
}


/*-----------------------------------------------------------------------------------------------*/


std::vector<Client*> const&		Channel::getUsers(void) const
{
	return (this->_users);
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
	this->_users.push_back(&user);
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


void	Channel::removeUser(const std::string& nick)
{
	for (size_t i = 0; i < this->_users.size(); i++)
	{
		if (this->_users[i]->getNick() == nick)
			this->_users.erase(this->_users.begin() + i);
	}
	// TODO : add message de sortie de channel
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


/*-----------------------------------------------------------------------------------------------*/


int Channel::checkUser(const std::string& nick)
{
	for (size_t i = 0; i < this->_users.size(); i++)
	{
		if (this->_users[i]->getNick() == nick)
			return (1);
	}
	return (0);
}

int Channel::checkOperator(const std::string& nick)
{
	for (size_t i = 0; i < this->_operators.size(); i++)
	{
		if (this->_users[i]->getNick() == nick)
			return (1);
	}
	return (0);
}

int Channel::checkInvite(const std::string& nick)
{
	for (size_t i = 0; i < this->_invite.size(); i++)
	{
		if (this->_invite[i].client->getNick() == nick)
			return (1);
	}
	return (0);
}


/*-----------------------------------------------------------------------------------------------*/


std::string	Channel::createStringUsers(void)
{
	std::string str;

	std::cout << this->_users.size() << std::endl;
	for (int i = 0; i < (int)this->_users.size(); ++i)
	{
		if (i > 0)
			str += " ";
		if (this->checkOperator(this->_users[i]->getNick()))
			str += "@";
		str += this->_users[i]->getNick();
	}
	return (str);
}

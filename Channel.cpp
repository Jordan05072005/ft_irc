#include "includes/header.hpp"

Channel::Channel(void){}

Channel::Channel(Channel const& copy)
{
	*this = copy;
	return ;
}

Channel& Channel::operator=(Channel const& other)
{
	if (this != &other){
		this->i = other.i;//faire une cpy profonde avec les getteur
	}
	return *this;
}

Channel::~Channel(void){}

Channel::Channel(std::string const& name, Client const& creator)
{
	this->name = name;
	this->operators.push_back(creator);
	this->users.push_back(creator);

	this->topic = "";
	this->channel_key = "";
	this->user_limit = 0; // no limit

	// TODO : voir les options par défaut et les différentes possibilités de création de channel en 1 ligne de commande
	this->i = false;
	this->t = false;
	this->k = false;
	this->o = false;
	this->l = false;
	return ;
}

int Channel::checkUser(const std::string& nick){
	for (size_t i = 0; i < this->users.size(); i++){
		if (this->users[i].getNick() == nick)
			return (1);
	}
	return (0);
}

void	Channel::delUsers(std::string& nick){
	for (size_t i =0; i < this->users.size(); i++){
		if (this->users[i].getNick() == nick)
		 users.erase(users.begin() + i);
	}
}

int Channel::checkOperator(const std::string& nick){
	for (size_t i = 0; i < this->operators.size(); i++){
		if (this->users[i].getNick() == nick)
			return (1);
	}
	return (0);
}

const std::vector<t_invitee>&	Channel::getInvitee()const{
	return (this->_invitee);
}


void Channel::addInvitee(Client& client){
	t_invitee invitee;

	invitee.client = &client;
	invitee.time = std::time(NULL);
	if (this->checkInvitee(client.getNick()))
		return ;
	this->_invitee.push_back(invitee);
}

int Channel::checkInvitee(const std::string& nick){
	for (size_t i = 0; i < this->_invitee.size(); i++){
		if (this->_invitee[i].client->getNick() == nick)
			return (1);
	}
	return (0);
}

void	Channel::delInvitee(){
	std::time_t now = std::time(NULL);
	for (size_t i = 0; i < this->_invitee.size(); i++){
		if (now - this->_invitee[i].time >= 600)
			this->_invitee.erase(this->_invitee.begin() + i);
	}
}

std::string const&	Channel::getName(void) const{
	return (this->name);
}

bool Channel::getOptInviteOnly(void) const{
	return (this->i);
}



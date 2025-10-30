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

int Channel::checkUser(std::string& nick){
	for (size_t i = 0; i < this->users.size(); i++){
		if (this->users[i].getNick() == nick)
			return (1);
	}
	return (0);
}

int Channel::checkOperator(std::string& nick){
	for (size_t i = 0; i < this->operators.size(); i++){
		if (this->users[i].getNick() == nick)
			return (1);
	}
	return (0);
}

#include <algorithm> // pour std::remove_if

void	Channel::delUsers(std::string& nick){
	for (size_t i =0; i < this->users.size(); i++){
		if (this->users[i].getNick() == nick)
		 users.erase(users.begin() + i);
	}
}

std::string const&	Channel::getName(void) const{
	return (this->name);
}


#include "includes/header.hpp"

Channel::Channel(void){}

Channel::Channel(Channel const& copy)
{
	this = copy;
	return ;
}

Channel::operator=(Channel const& other)
{
	this = other;
	return ;
}

Channel::~Channel(void){}

Channel::Channel(std::string const& name, Client const& creator)
{
	this->name = name;
	this->operators.push_back(creator);
	this->users.push_back(creator);

	this->topic = nullptr;
	this->channel_key = nullptr;
	this->user_limit = 0; // no limit

	// TODO : voir les options par défaut et les différentes possibilités de création de channel en 1 ligne de commande
	this->i = false;
	this->t = false;
	this->k = false;
	this->o = false;
	this->l = false;
	return ;
}

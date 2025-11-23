
#include "../includes/header.hpp"

int	Server::checkUniqueNick(std::string const& nick)
{
	for (size_t i = 0; i < this->_clients.size(); i++)
	{
		if (this->_clients[i]->getNick() == nick)
			return 1;
	}
	return 0;
}

int Server::checkExistClient(std::string const& nick)
{
	for (size_t i = 0; i < this->_clients.size(); i++)
	{
		if (this->_clients[i]->getNick() == nick)
			return 1;
	}
	return 0;
}

int	Server::checkExistChannel(std::string const& name)
{
	for (size_t i = 0; i < this->_channel.size(); i++)
	{
		if (this->_channel[i]->getName() == name)
			return (1);
	}
	return (0);
}

int Server::autorisedNick(std::string& name){
	std::string autorised_start = "[]\\_^{ }|`";
	std::string autorised = "-[]\\`_^{}|";
	if (name.size() > 30)
		return (0);
	for (size_t i = 0; i < name.size(); i++){
		if (i == 0 
			&& autorised_start.find(name[i]) == std::string::npos
			&& !(name[i] >= 'A' && name[i] <= 'Z')
			&& !(name[i] >= 'a' && name[i] <= 'z')
		)
			return (0);
		else if (i != 0
			&& autorised.find(name[i]) == std::string::npos
			&& !(name[i] >= 'A' && name[i] <= 'Z')
			&& !(name[i] >= 'a' && name[i] <= 'z')
			&& !(name[i] >= '0' && name[i] <= '9')
		)
			return (0);
	}
	return (1);
}

int Server::autorisedIdent(std::string& name){
	if (name.size() > 30)
		return (0);
	for (size_t i = 0; i < name.size(); i++){
		if (i != 0
			&& name[i] != '_'
			&& !(name[i] >= 'A' && name[i] <= 'Z')
			&& !(name[i] >= 'a' && name[i] <= 'z')
			&& !(name[i] >= '0' && name[i] <= '9')
		)
			return (0);
	}
	return (1);
}

int Server::autorisedRealName(std::string& name){
	if (name.size() > 30)
		return (0);
	for (size_t i = 0; i < name.size(); i++){
		if (i != 0 && name[i] == ':')
			return (0);
	}
	return (1);
}

/*-----------------------------------------------------------------------------------------------*/


int	Server::getIndexChannel(std::string const& name)
{
	for (size_t i = 0; i < this->_channel.size(); i++)
	{
		if (this->_channel[i]->getName() == name)
			return (i);
	}
	return (0);
}

Client&	Server::getClient(std::string const& nick)
{
	for (size_t i = 0; i < this->_clients.size(); i++)
	{
		if (this->_clients[i]->getNick() == nick)
			return (*(this->_clients[i]));
	}
	return (*(this->_clients[0])); // TODO : trouver solution pr null
}


/*-----------------------------------------------------------------------------------------------*/


// supprime le client et son socket de partout
void Server::delClient(int index)
{
	Client*	tmp = this->_clients[index - 1];

	close(this->_clients[index - 1]->getFd()); // ferme client socket
	this->_clients.erase(this->_clients.begin() + (index - 1)); // retire le client du gestionnaire
	delete tmp;
	this->_fds.erase(this->_fds.begin() + index); // retire le client socket des sockets actifs
	
	return ;
}

void Server::delInvite(void)
{
	for (size_t i = 0; i < this->_channel.size(); i++)
	{
		if (!this->_channel[i]->getInvite().empty())
			this->_channel[i]->removeInvite();
	}
	return ;
}



//:Alice!alice@host PART #general :Bye everyone!
void	Server::delAllChannelClient(Client& client, std::string& cmd, std::string mess)
{
	std::vector<Channel*> c;
	std::vector<std::string> names;

	c = client.getChannels();
	names.push_back(client.getNick());
	for (size_t i = 0; i < this->_clients.size(); i++)
	{
		for (size_t j = 0; j < c.size(); j++)
		{
			if (c[j]->checkUser(this->_clients[i]->getNick())
				&& std::find(names.begin(), names.end(), this->_clients[i]->getNick()) == names.end())
			{
				this->sendMessUser(client, *this->_clients[i], cmd, mess);
				names.push_back(this->_clients[i]->getNick());
				break;
			}
		}
	}
	std::cout << "mid" << std::endl;
	std::cout << c.size() << std::endl;
	for (size_t i = 0; i < this->_channel.size(); i++)
	{
		this->_channel[i]->removeUser(client.getNick());
		this->_channel[i]->removeOperator(client.getNick());
		// this->sendMessChannel((*it)->getName(), (*it)->getName(), cmd, mess, client);
		if (this->_channel[i]->getUsers().size() == 0)
		{
			delete this->_channel[i];
			this->_channel.erase(this->_channel.begin() + i);
			return ;
		}
	}
	client.removeAllChannels();
	return ;
}


/*-----------------------------------------------------------------------------------------------*/


std::string	Server::createStringChannels(void) const
{
	std::string str;

	for (size_t i = 0; i < this->_channel.size(); i++)
	{
		if (i != 0)
			str += ',';
		str += this->_channel[i]->getName();
	}
	return (str);
}

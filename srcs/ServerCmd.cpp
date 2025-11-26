#include "../includes/header.hpp"


int	Server::checkCap(Client& client, std::vector<std::string>& mess)
{
	std::string message;

	if (mess.size() == 1)
		return (0);
	if (mess[1] == "LS")
	{
		message = ":irc CAP * LS :\r\n";
		send(client.getFd(), message.c_str(), message.size(), 0);
	}
	else if (mess[1] == "END")
	{
		if (!client.getIdent().empty() && !client.getNick().empty())
		{
			client.setState(client.getState() + 1);
			this->sendMessLocal("001", "", client, "Welcome to the IRC Network");
		}
	}
	return 0;
}

int Server::checkPass(Client& client, std::vector<std::string>& mess)
{
	std::string err;

	if (mess.size() == 1)
		return (this->sendMessLocal("461", mess[0], client, "Not enough parameters"), 0);
	if (client.getState() >= 1)
		return (this->sendMessLocal("462", "", client, "Unauthorized command (already registered)"), 0);
	if (mess[1] != this->_password)
		return (this->sendMessLocal("464", "", client, "Incorrect password"), 1);

	client.setState(client.getState() + 1);
	return (0);
}

int Server::checkNick(Client& client, std::vector<std::string>& mess)
{
	if (!client.getNick().empty() && client.getState() >= 2)
		return (this->sendMessLocal("462", "", client, "Unauthorized command (already registered)"), 0);
	if (mess.size() == 1)
		return (this->sendMessLocal("431", "", client, "No nickname given"), 0);
	if (this->checkUniqueNick(mess[1]))
		return (this->sendMessLocal("433", "", client, "Nickname is already in use"), 0);
	if (!this->autorisedNick(mess[1]))
		return (this->sendMessLocal("432", mess[1], client, "Erroneous nickname"), 0);
	client.setNick(mess[1]);
	return 0;
}

int Server::checkUser(Client& client, std::vector<std::string>& mess)
{
	std::string realname;

	if ((!client.getRealName().empty() || !client.getIdent().empty()) && client.getState() >= 2)
		return (this->sendMessLocal("462", "", client, "Unauthorized command (already registered)"), 0);
	if (mess.size() < 5)
		return (this->sendMessLocal("461", "", client, "Not enough parameters"), 0);
	if (mess[4][0] != ':')
		return (this->sendMessLocal("461", "", client, "Not enough parameters"), 0);
	client.setIdent(mess[1]);
	for (size_t i = 4; i < mess.size(); i++)
		realname += mess[i];
	if (!this->autorisedIdent(mess[1]) || !this->autorisedRealName(realname))
		return (this->sendMessLocal("461", "", client, "Not enough parameters"), 0);
	client.setIdent(mess[1]);
	client.setRealName(realname.erase(0, 1));
	return 0;
}

int Server::checkQuit(Client& client, std::vector<std::string>& mess)
{
	std::string message;

	if (mess.size() == 1)
		message = "Client Quit";
	else
	{
		if (mess[1][0] != ':')
			message = mess[1];
		else
		{
			for (size_t i = 1; i < mess.size(); i++)
				message += mess[i];
			message.erase(0, 1);
		}
	}
	this->delAllChannelClient(client, mess[0], message);
	if (client.getState() < 2)
		return (1);
	return (1);
}

//KICK <channel> <user> [<comment>]
// :<kicker_nick>!<user>@<host> KICK <channel> <target_nick> :<comment>
// :Alice!~alice@127.0.0.1 KICK #42school johon :Too much spam
// :server 461 <nick> KICK :Not enough parameters
int	Server::checkKick(Client& client, std::vector<std::string>& mess)
{
	std::string message;
	Channel* channel;
	int	i;

	if (mess.size() < 3)
		return (this->sendMessLocal("461", mess[0], client, "Not enough parameters"), 0);
	if (!this->checkExistChannel(mess[1]))
		return (this->sendMessLocal("403", mess[1], client, "No such channel"), 0);
	channel = this->_channel[this->getIndexChannel(mess[1])];
	if (!channel->checkUser(client.getNick()))
		return (this->sendMessLocal("442", mess[1], client, "You're not on that channel"), 0);
	if (!channel->checkOperator(client.getNick()))
		return (this->sendMessLocal("482", mess[1], client, "You're not channel operator"), 0);
	if (!channel->checkUser(mess[2]))
		return (this->sendMessLocal("441", mess[2] + " " + mess[1], client, "They aren't on that channel"), 0);
	if (mess.size() == 3)
		message = "";
	else if (mess[3][0] == ':')
	{
		for (size_t i = 3; i < mess.size(); i++)
			message += (mess[i] + ((i + 1) < mess.size()  ? " " : ""));
		message.erase(0, 1);
	}
	else
		message = mess[3];
	this->sendMessChannel(mess[1], mess[0] + " " + mess[1] + " " + mess[2], message, 1, client);
	channel->removeUser(mess[2]);
	channel->removeOperator(mess[2]);
	client.removeChannel(mess[1]);
	if (channel->getUsers().size() == 0)
	{
		i = this->getIndexChannel(channel->getName());
		delete this->_channel[i];
		this->_channel.erase(this->_channel.begin() + i);
	}
	this->getClient(mess[2]).removeChannel(mess[1]);
	return (0);
}

//INVITE <nickname> <channel>
int Server::checkInvite(Client& client, std::vector<std::string>& mess)
{
	Channel* channel;
	int err = 0;

	if (mess.size() < 3)
		return (this->sendMessLocal("461", mess[0], client, "Not enough parameters"), 0);
	if (!this->checkExistClient(mess[1]) && err++)
		this->sendMessLocal("401", mess[1], client, "No such nick/channel");
	if (!this->checkExistChannel(mess[2]) && err++)
		this->sendMessLocal("403", mess[2], client, "No such channel");
	if (err > 0)
			return 0;
	channel = this->_channel[this->getIndexChannel(mess[2])];
	if (!channel->checkUser(client.getNick()))
		return (this->sendMessLocal("442", mess[2], client, "You're not on that channel"), 0);
	if (channel->getOptInviteOnly() && !channel->checkOperator(client.getNick()))
		return (this->sendMessLocal("482", mess[2], client, "You're not channel operator"), 0);
	if (channel->checkUser(mess[1]))
		return (this->sendMessLocal("443", client.getNick() + " " + mess[1] + " " + mess[2], client, "is already on channel"), 0);
	this->sendMessLocal("341", mess[1] + " " + mess[2], client, "");
	this->sendMessLocal("", mess[0], this->getClient(mess[1]), mess[2]);
	channel->addInvite(this->getClient(mess[1]));
	return (0);
}

// TOPIC <Channel> [<newTopic>]
int Server::checkTopic(Client& client, std::vector<std::string>& mess)
{
	Channel* channel;
	std::string message;
	t_topic t;


	if (mess.size() < 2)
		return (this->sendMessLocal("461", mess[0], client, "Not enough parameters"), 0);
	if (!this->checkExistChannel(mess[1]))
		return (this->sendMessLocal("403", mess[1], client, "No such channel"), 0);
	channel = this->_channel[this->getIndexChannel(mess[1])];
	if (!channel->checkUser(client.getNick()))
		return (this->sendMessLocal("442", mess[1], client, "You're not on that channel"), 0);
	if (channel->getOptRestrictTopic()
		&& !channel->checkOperator(client.getNick()))
		return (this->sendMessLocal("482", mess[2], client, "You're not channel operator"), 0);
	if (mess.size() == 2)
	{
		if (channel->getTopic().topic.empty())
			return (this->sendMessLocal("331", mess[1], client, "No topic is set"), 0);
		t = channel->getTopic();
		this->sendMessLocal("332", mess[1], client, t.topic);
		return (this->sendMessLocal("333", mess[1] + " " + t.modifBy + " " + convertTimeStr(t.time), client, ""), 0);
	}
	if (mess[2][0] != ':')
		message = mess[2];
	else
	{
		for (size_t i = 2; i < mess.size(); i++)
			message += (mess[i] + " ");
		message.erase(0, 1);
	}
	channel->setTopic(message, client.getNick());
	return(this->sendMessChannel(mess[1], mess[0] + " " + mess[1], message, 1, client), 0);
}

static int	is_name_ok(std::string const& name)
{
	if (name.size() < 2 || name[0] != '#' || name.size() > 50)
		return (0);
	for (size_t i = 0; i < name.size(); i++)
	{
		if (name[i] == ' ' || name[i] == ',' || name[i] == ':' || name[i] == 7 || name[i] == 13 || name[i] == 10)
			return (0);
	}
	return (1);
}

// JOIN [#channel,&channel] [key,key]
// :Alice!~alice@host JOIN :#test
int		Server::checkJoin(Client& client, std::vector<std::string>& mess)
{
	if (mess.size() < 2 || mess.size() > 3)
		return (this->sendMessLocal("461", mess[0], client, "Not enough parameters"), 0);
	if (mess.size() == 2 && mess[1] == "0")
	{
		std::vector<std::string>	cmd;
		cmd.push_back("PART");
		cmd.push_back(this->createStringChannels());
		return (this->checkPart(client, cmd), 0);
	}

	// separation of many commands into one at a time
	std::vector<std::string> mess_cpy = mess;
	if (mess_cpy[1].find(',') != std::string::npos || (mess_cpy.size() == 3 && mess_cpy[2].find(',') != std::string::npos))
	{
		std::vector<std::string> parameters;

		while (mess_cpy[1].find(',') != std::string::npos)
		{
			parameters.push_back(mess_cpy[0]); // "JOIN"
			parameters.push_back(mess_cpy[1].substr(0, mess_cpy[1].find(',')));
			mess_cpy[1] = mess_cpy[1].substr(mess_cpy[1].find(',') + 1);
			if (mess_cpy.size() == 3 && mess_cpy[2].find(',') != std::string::npos)
			{
				if (mess_cpy[2][0] != ',')
					parameters.push_back(mess_cpy[2].substr(0, mess_cpy[2].find(',')));
				mess_cpy[2] = mess_cpy[2].substr(mess_cpy[2].find(',') + 1);
			}
			else if (mess_cpy.size() == 3)
			{
				parameters.push_back(mess_cpy[2]);
				mess_cpy.pop_back();
			}
			this->checkJoin(client, parameters);
			parameters.clear();
		}
		if (mess_cpy.size() == 3 && mess_cpy[2].find(',') != std::string::npos && mess_cpy[2][0] != ',')
			mess_cpy[2] = mess_cpy[2].substr(0, mess_cpy[2].find(','));
	}

	Channel*	channel;
	bool		is_created = false;

	// check channel availability
	if (!this->checkExistChannel(mess_cpy[1]))
	{
		// creating channel at the norm
		if (!is_name_ok(mess_cpy[1]))
			return (this->sendMessLocal("476", mess_cpy[1], client, "Bad Channel Mask"), 0);
		if (mess_cpy.size() == 3)
			this->_channel.push_back(new Channel(ft_tolower(mess_cpy[1]), mess_cpy[2], &client));
		else
			this->_channel.push_back(new Channel(ft_tolower(mess_cpy[1]), &client));
		channel = this->_channel[this->_channel.size() - 1];
		is_created = true;
	}
	else
		channel = this->_channel[this->getIndexChannel(mess_cpy[1])];

	// check if client already on
	if (!is_created && channel->checkUser(client.getNick()))
		return (this->sendMessLocal("443", channel->getName(), client, "is already on channel"), 0);

	// check invite-only
	bool	is_invite = false;
	if (!is_created && channel->getOptInviteOnly() == true)
	{
		std::vector<t_invite> const	invite = channel->getInvite();

		for (size_t i = 0; i < invite.size(); ++i)
		{
			if (client.getNick() == invite[i].client->getNick())
			{
				is_invite = true;
				break;
			}
		}

		if (is_invite == false)
			return (this->sendMessLocal("473", mess_cpy[1], client, "Cannot join channel (+i)"), 0);
	}

	// check channel_key
	if (!is_created && channel->getOptChannelKey() == true && (mess_cpy.size() != 3 || (mess_cpy.size() == 3 && channel->getChannelKey() != mess_cpy[2])))
		return (this->sendMessLocal("475", mess_cpy[1], client, "Cannot join channel (+k)"), 0);

	// check user limit
	if (!is_created && channel->getOptUserLimit() == true && channel->getUsersCountNb() >= channel->getUserLimit())
		return (this->sendMessLocal("471", mess_cpy[1], client, "Cannot join channel (+l)"), 0);

	// welcome messages
	if (!is_created)
	{
		client.addChannel(this->_channel[this->getIndexChannel(mess_cpy[1])]);
		channel->addUser(client);
	}

	if (is_invite)
		channel->delInvite(client);
	this->sendMessChannel(mess_cpy[1], mess_cpy[0], mess_cpy[1], 1, client);
	if (!channel->getTopic().topic.empty())
	{
		this->sendMessLocal("332", mess_cpy[1], client, channel->getTopic().topic);
		this->sendMessLocal("333", mess_cpy[1] + " " + channel->getTopic().modifBy + " " + convertTimeStr(channel->getTopic().time), client, "");
	}
	else
		this->sendMessLocal("331", mess_cpy[1], client, "No topic is set");
	this->sendMessLocal("353", "= " + mess_cpy[1], client, channel->createStringUsers());
	this->sendMessLocal("366", mess_cpy[1], client, "End of /NAMES list");
	return(0);
}

static bool	is_userlimit_ok(std::string const& str)
{
	for (size_t i = 1; i < str.size(); i++)
	{
		if (!std::isdigit(str[i]))
			return (false);
	}
	return (true);
}

/*
MODE #channel => request, response : 
	:irc.local.net 324 Alice #channel +itk secret
	:irc.local.net 329 Alice #channel 1730910842

--- CLASSIC ---
MODE #channel +itklo secret 10 <client> => request, response always in this syntax
MODE #channel +itkl-o secret 10 <client> => request, response always in this syntax
MODE #channel +i +t +k secret +l 10 +o <client> => demande
MODE #channel +i +t +kl secret 10 +o <client> => demande
MODE #channel -option <client> => demande

--- UNUSUAL ---
MODE #channel ++ => do nothing
MODE #channel ++i => understood like +i
MODE #channel +-i => understood like -i
MODE #channel +t+-i => understood like +t et -i
MODE #channel poulain => ignored
MODE #channel +i <arg> => arg ignored
MODE #channel +i +t +k secret poulain +l 10 +o <client> cage => poulain and cage ignored
MODE #channel +kti secret poulain +l 10 +o <client> cage => poulain and cage ignored
*/
int		Server::checkMode(Client& client, std::vector<std::string>& mess)
{
	if (mess.size() < 2)
		return (this->sendMessLocal("461", mess[0], client, "Not enough parameters"), 0);

	if (mess.size() > 2)
	{
		std::vector<std::string>	mess_cpy = mess;

		// separation into distinct parameters
		std::vector<std::string>	parameters;
		for (std::vector<std::string>::iterator	it = mess_cpy.begin() + 2; it < mess_cpy.end(); it++) // we ignore MODE #channel
		{
			// is modestring ?
			if ((*it)[0] == '+' || (*it)[0] == '-')
			{
				// separate modestring
				std::vector<std::string>	tmp;
				std::string					what;
				std::string					mode;
				for (size_t i = 0; i < (*it).size(); i++)
				{
					if ((*it)[i] == '+' || (*it)[i] == '-')
						what = (*it)[i];
					else
					{
						mode = what + (*it)[i];
						tmp.push_back(mode);
					}
				}
				if (!tmp.empty())
					parameters.insert(parameters.end(), tmp.begin(), tmp.end());
			}
			else
				parameters.push_back(*it);
		}

		if (parameters.empty())
			return (0);
		
		if (!this->checkExistChannel(mess[1]))
			return (this->sendMessLocal("403", mess[1], client, "No such channel"), 0);

		Channel*	channel = this->_channel[this->getIndexChannel(mess[1])];

		if (!channel->checkUser(client.getNick()))
			return (this->sendMessLocal("442", mess[1], client, "You're not on that channel"), 0);

		if (!channel->checkOperator(client.getNick()))
			return (this->sendMessLocal("482", mess[1], client, "You're not channel operator"), 0);

		// execute MODE
		std::vector<std::string>	done;
		for (std::vector<std::string>::iterator it = parameters.begin(); it != parameters.end(); it++)
		{
			if ((*it)[0] == '+' || (*it)[0] == '-')
			{
				std::vector<std::string>::iterator	arg;
				bool								is_arg = false;
				bool								is_err = false;

				// does mode need arg ?
				if ((*it) == "+k" || (*it) == "+l" || (*it) == "+o" || (*it) == "-o")
				{
					for (arg = it; arg != parameters.end(); arg++)
					{
						if ((*arg)[0] != '+' && (*arg)[0] != '-')
							break;
					}
					if (arg == parameters.end())
					{
						is_err = true;
						this->sendMessLocal("461", mess[0], client, "Not enough parameters");
					}
					else
						is_arg = true;
				}

				// change mode
				if (*it == "+i")
					channel->setOptInviteOnly(true);
				else if (*it == "+t")
					channel->setOptRestrictTopic(true);
				else if (is_err == false && *it == "+k")
				{
					if (channel->getOptChannelKey() == true && *arg == channel->getChannelKey())
					{
						is_err = true;
						this->sendMessLocal("467", mess[1], client, "Channel key already set");
					}
					else
					{
						channel->setOptChannelKey(true);
						channel->setChannelKey(*arg);
					}
				}
				else if (is_err == false && *it == "+l")
				{
					if (!is_userlimit_ok(*arg))
					{
						is_err = true;
						this->sendMessLocal("461", mess[0], client, "Not enough parameters");
					}
					else
					{
						channel->setOptUserLimit(true);
						channel->setUserLimit(std::atoi((*arg).c_str()));
					}
				}
				else if (is_err == false && *it == "+o")
				{
					if (!this->checkExistClient(*arg))
					{
						is_err = true;
						this->sendMessLocal("401", *arg, client, "No such nick/channel");
					}
					if (is_err == false && !channel->checkUser(*arg))
					{
						is_err = true;
						this->sendMessLocal("441", *arg + " " + mess[1], client, "They aren't on that channel");
					}
					else if (is_err == false && !channel->checkOperator(*arg))
						channel->addOperator(this->getClient(*arg));
				}
				else if (*it == "-i")
					channel->setOptInviteOnly(false);
				else if (*it == "-t")
					channel->setOptRestrictTopic(false);
				else if (*it == "-k")
				{
					channel->setOptChannelKey(false);
					channel->setChannelKey("");
				}
				else if (*it == "-l")
				{
					channel->setOptUserLimit(false);
					channel->setUserLimit(0);
				}
				else if (is_err == false && *it == "-o")
				{
					if (!this->checkExistClient(*arg))
					{
						is_err = true;
						this->sendMessLocal("401", *arg, client, "No such nick/channel");
					}
					if (is_err == false && !channel->checkUser(*arg))
					{
						is_err = true;
						this->sendMessLocal("441", *arg + " " + mess[1], client, "They aren't on that channel");
					}
					else if (is_err == false && channel->checkOperator(*arg))
						channel->removeOperator(*arg);
				}
				else
				{
					size_t i;
					for (i = 0; i < (*it).size(); i++)
					{
						if (std::isalpha((*it)[i]))
							break ;
					}
					this->sendMessLocal("472", (*it).substr(i, 1), client, "is unknown mode char to me");
				}

				if (is_err == false)
					done.push_back(*it);
				if (is_arg)
				{
					if (is_err == false)
						done.push_back(*arg);
					parameters.erase(arg);
				}
			}
		}

		if (done.empty())
			return (0);

		// construct response for client
		std::vector<std::string>	order;
		order.push_back("+i");
		order.push_back("+t");
		order.push_back("+k");
		order.push_back("+l");
		order.push_back("+o");

		order.push_back("-i");
		order.push_back("-t");
		order.push_back("-k");
		order.push_back("-l");
		order.push_back("-o");

		mess_cpy.clear();
		for (std::vector<std::string>::iterator ito = order.begin(); ito != order.end(); ++ito)
		{
			for (std::vector<std::string>::iterator itd = done.begin(); itd != done.end(); ++itd)
			{
				if (*itd == *ito)
				{
					if (mess_cpy.size() >= 1)
						mess_cpy[0] = add_to_modestring(mess_cpy[0], *itd);
					else
						mess_cpy.push_back(*itd);
					if ((*itd) == "+k" || (*itd) == "+l" || (*itd) == "+o" || (*itd) == "-o")
					{
						mess_cpy.push_back(*(itd + 1));
						done.erase(itd + 1);
					}
					break ;
				}
			}
		}

		std::string	response = "";

		// we skip MODE #channel
		for (std::vector<std::string>::iterator itm = mess_cpy.begin(); itm != mess_cpy.end(); itm++)
		{
			if (itm != mess_cpy.begin())
				response += ' ';
			response += *itm;
		}
		this->sendMessChannel(mess[1], mess[0] + " " + mess[1] + " " + response, "", 1, client);
	}
	else
	{
		if (!this->checkExistChannel(mess[1]))
			return (this->sendMessLocal("403", mess[1], client, "No such channel"), 0);

		Channel*	channel = this->_channel[this->getIndexChannel(mess[1])];

		if (!channel->checkUser(client.getNick()))
			return (this->sendMessLocal("442", mess[1], client, "You're not on that channel"), 0);

		this->sendMessLocal("324", mess[0] + " " + channel->createStringModes(), client, "");
		this->sendMessLocal("329", mess[0] + " " + convertTimeStr(channel->getCreationTime()), client, "");
	}
	return (0);
}

// PART #channel,#channel <reason>
// :nick!user@host PART #channel :reason
// :nick!user@host PART #channel
int		Server::checkPart(Client& client, std::vector<std::string>& mess)
{
	if (mess.size() < 2)
		return (this->sendMessLocal("461", mess[0], client, "Not enough parameters"), 0);

	// separation of many commands into one at a time
	std::vector<std::string> mess_cpy = mess;
	if (mess_cpy[1].find(',') != std::string::npos)
	{
		std::vector<std::string> parameters;

		while (mess_cpy[1].find(',') != std::string::npos)
		{
			parameters.push_back(mess_cpy[0]); // "PART"
			parameters.push_back(mess_cpy[1].substr(0, mess_cpy[1].find(',')));
			mess_cpy[1] = mess_cpy[1].substr(mess_cpy[1].find(',') + 1);
			for (size_t i = 2; i < mess_cpy.size(); i++)
				parameters.push_back(mess_cpy[i]);
			this->checkPart(client, parameters);
			parameters.clear();
		}
	}
	
	if (!this->checkExistChannel(mess_cpy[1]))
		return (this->sendMessLocal("403", mess_cpy[1], client, "No such channel"), 0);

	Channel*	channel = this->_channel[this->getIndexChannel(mess_cpy[1])];

	if (!channel->checkUser(client.getNick()))
		return (this->sendMessLocal("442", mess_cpy[1], client, "You're not on that channel"), 0);
	
	std::string	reason;
	if (mess_cpy.size() < 3)
		reason = "";
	else if (mess_cpy[2][0] != ':')
		reason = mess_cpy[2];
	else
	{
		for (size_t i = 2; i < mess_cpy.size(); i++)
		{
			if (i != 2)
				reason += ' ';
			reason += mess_cpy[i];
		}
		reason.erase(0, 1);
	}
	this->sendMessChannel(mess_cpy[1], mess_cpy[0] + " " + mess_cpy[1], reason, 0, client);

	client.removeChannel(mess_cpy[1]);
	channel->removeUser(client.getNick());

	// check if channel empty
	if (channel->getUsers().size() == 0)
	{
		int i = this->getIndexChannel(channel->getName());
		delete this->_channel[i];
		this->_channel.erase(this->_channel.begin() + i);
	}

	return (0);
}

// DONT SEND ERRORS JUST DO NOTHING
// NOTICE <msgtarget> <mess>
// if target is channel, all users receives apart himself
int		Server::checkNotice(Client& client, std::vector<std::string>& mess)
{
	if (mess.size() < 3)
		return (0);
	
	std::string message;
	for (size_t i = 2; i < mess.size(); i++)
	{
		if (i != 2)
			message += ' ';
		message += mess[i];
	}
	message.erase(0, 1);

	if (this->checkExistClient(mess[1])) // send mess to someone
	{
		this->sendMessUser(client, this->getClient(mess[1]), mess[0], message);
	}
	else if (this->checkExistChannel(mess[1])) // send mess to channel
	{
		Channel*	channel = this->_channel[this->getIndexChannel(mess[1])];

		if (channel->checkUser(client.getNick())) // if user in channel
			this->sendMessChannel(mess[1], mess[0] + " " + mess[1], message, 0, client);
	}
	return (0);
}

// LIST
// LIST #channel,#channel
int		Server::checkList(Client& client, std::vector<std::string>& mess)
{
	this->sendMessLocal("321", "Channel", client, "Users Name");

	if (mess.size() > 1)
	{
		// separation of many channels into one at a time
		std::vector<std::string>	mess_cpy = mess;
		Channel*					channel;
		while (mess_cpy[1].find(',') != std::string::npos)
		{
			if (this->checkExistChannel(mess_cpy[1].substr(0, mess_cpy[1].find(','))))
			{
				channel = this->_channel[this->getIndexChannel(mess_cpy[1].substr(0, mess_cpy[1].find(',')))];
				if (channel->getTopic().topic.empty())
					this->sendMessLocal("322", channel->getName() + " " + channel->getUsersCountStr(), client, " ");
				else
					this->sendMessLocal("322", channel->getName() + " " + channel->getUsersCountStr(), client, channel->getTopic().topic);
			}
			mess_cpy[1] = mess_cpy[1].substr(mess_cpy[1].find(',') + 1);
		}
		if (this->checkExistChannel(mess_cpy[1]))
		{
			channel = this->_channel[this->getIndexChannel(mess_cpy[1])];
			if (channel->getTopic().topic.empty())
				this->sendMessLocal("322", channel->getName() + " " + channel->getUsersCountStr(), client, " ");
			else
				this->sendMessLocal("322", channel->getName() + " " + channel->getUsersCountStr(), client, channel->getTopic().topic);
		}
	}
	else
	{
		for (std::vector<Channel*>::const_iterator	it = this->_channel.begin(); it != this->_channel.end(); it++)
		{
			if ((*it)->getTopic().topic.empty())
				this->sendMessLocal("322", (*it)->getName() + " " + (*it)->getUsersCountStr(), client, " ");
			else
				this->sendMessLocal("322", (*it)->getName() + " " + (*it)->getUsersCountStr(), client, (*it)->getTopic().topic);
		}
	}

	this->sendMessLocal("323", "", client, "End of /LIST");
	return (0);
}


//PRIVMSG <receiver>{,<receiver>} :<text to be sent>
int		Server::checkPrivmsg(Client& client, std::vector<std::string>& mess)
{
	std::vector<std::string> argm;
	std::string message;
	std::vector<std::string> mess_cpy;

	if (mess.size() < 2)
		return (this->sendMessLocal("461", mess[0], client, "Not enough parameters"), 0);
	if (mess.size() < 3 || mess[2][0] != ':' || mess[2].size() == 1)
		return (this->sendMessLocal("412", "", client, "No text to send"), 0);
	if (client.isMute())
			return (sendMessBot(*this->_bot[0], client, "NOTICE", this->_bot[0]->getMessMute()), 0);
	argm = split(mess[1], ',');
	mess[2].erase(0, 1);
	for (size_t i = 2; i < mess.size(); i++)
		message = message + (mess[i] + ((i + 1) == mess.size() ? "" : " "));
	if (this->_bot[0]->checkMessage(message)){
		client.addWarn();
		return (sendMessBot(*this->_bot[0], client, "NOTICE", this->_bot[0]->getMessBadWords()), 0);
	}
	for (size_t i = 0; i < this->_cmd.size(); i++)
	{
		if (message == this->_cmd[i].name)
		{
			mess_cpy = mess;
			mess_cpy.erase(mess_cpy.begin(), mess_cpy.begin() + 2);
			return((this->*(_cmd[i].pars))(client, mess_cpy));
		}
	}
	for (size_t i = 0; i < argm.size(); i++)
	{
		if (argm[i][0] != '#')
		{
			if (!this->checkExistClient(argm[i]))
				this->sendMessLocal("401", argm[i], client, "No such nick/channel");
			else
				this->sendMessUser(client, this->getClient(argm[i]), mess[0], message);
		}
		else
		{
			if (!this->checkExistChannel(argm[i]))
				this->sendMessLocal("403", argm[i], client, "No such channel");
			else if (!this->_channel[this->getIndexChannel(argm[i])]->checkUser(client.getNick()))
				this->sendMessLocal("442", argm[i], client, "You're not on that channel");
			else
				this->sendMessChannel(argm[i], mess[0] + " " + argm[i], message, 0, client);
		}
	}
	return (0);
}

int Server::checkNames(Client& client, std::vector<std::string>& mess)
{
	std::vector<std::string> channels;

	if (mess.size() == 1)
	{
		for (size_t i = 0; i < this->_channel.size(); i++)
		{
			this->sendMessLocal("353", "= " + this->_channel[i]->getName(), client, this->_channel[i]->createStringUsers());
			this->sendMessLocal("366", this->_channel[i]->getName(), client, "End of /NAMES list");
		}
		return (0);
	}
	channels = split(mess[1], ',');
	for (size_t i = 0; i < channels.size(); i++)
	{
		if (!this->checkExistChannel(channels[i]))
			this->sendMessLocal("403", channels[i], client, "No such channel");
		else
		{
			this->sendMessLocal("353", "= " + channels[i], client, this->_channel[this->getIndexChannel(channels[i])]->createStringUsers());
			this->sendMessLocal("366", channels[i], client, "End of /NAMES list");
		}
	}
	return (0);
}

int		Server::checkWho(Client& client, std::vector<std::string>& mess)
{
	std::vector<std::string> channels;
	std::vector<Client*> clients;

	if (mess.size() == 1)
	{
		for (size_t i = 0; i < this->_clients.size(); i++)
		{
			this->sendMessLocal("352", "* " + this->_clients[i]->getIdent() + " " +
				this->_clients[i]->getHost() + " " + 
				this->_clients[i]->getServ() + " " + 
				this->_clients[i]->getNick() + " H", client, "0 " + this->_clients[i]->getRealName());
		}
		this->sendMessLocal("315", "*", client, "End of /WHO list");
		return (0);
	}
	channels = split(mess[1], ',');
	for (size_t i = 0; i < channels.size(); i++)
	{
		if (channels[i][0] == '#' && !this->checkExistChannel(channels[i]))
			this->sendMessLocal("403", channels[i], client, "No such channel");
		else if (channels[i][0] != '#' && !this->checkExistClient(channels[i]))
			this->sendMessLocal("401", channels[i], client, "No such nick");
		else if (channels[i][0] != '#'){
			this->sendMessLocal("352", "* " + this->getClient(channels[i]).getIdent() + " " +
				this->getClient(channels[i]).getHost() + " " + 
				this->getClient(channels[i]).getServ() + " " + 
				this->getClient(channels[i]).getNick() + " H", client, "0 " + 
				this->getClient(channels[i]).getRealName());
			this->sendMessLocal("315", channels[i], client, "End of /WHO list");
		}
		else
		{
			clients = this->_channel[this->getIndexChannel(channels[i])]->getUsers();
			for (size_t j = 0; j < clients.size(); j++)
			{
				this->sendMessLocal("352", channels[i]  + " " + clients[j]->getIdent() + " " +
					clients[j]->getHost() + " " + 
					clients[j]->getServ() + " " + 
					clients[j]->getNick() + " H", client, "0 " + 
					clients[j]->getRealName());
			}
			this->sendMessLocal("315", channels[i], client, "End of /WHO list");
		}
	}
	return (0);
}

/*
	3️⃣ RPL_WHOISIDLE — 317
	:irc.example.com 317 <requester> <nickname> <seconds idle> :seconds idle


	<seconds idle> → temps depuis la dernière activité du client en secondes

	4️⃣ RPL_WHOISCHANNELS — 319
	:irc.example.com 319 <requester> <nickname> :<channel list>


	<channel list> → canaux dans lesquels l’utilisateur est actuellement connecté, séparés par des espaces

	5️⃣ RPL_ENDOFWHOIS — 318
	:irc.example.com 318 <requester> <nickname> :End of WHOIS list


	Terminer la réponse WHOIS, obligatoire

	<nickname> → pseudo interrogé
*/
int		Server::checkWhois(Client& client, std::vector<std::string>& mess){
	Client *c;
	std::stringstream ss;
	if (mess.size() == 1)
		return (this->sendMessLocal("431", mess[0], client, "No nickname given") , 0);
	if (!this->checkExistClient(mess[1]))
		return (this->sendMessLocal("401", mess[1], client, "No such nick") , 0);
	c = &this->getClient(mess[1]);
	this->sendMessLocal("311", c->getNick() + " ~" + c->getIdent() + " " + c->getHost() + " * " , client, c->getRealName());
	this->sendMessLocal("312", c->getNick() + " " + c->getHost() + " " , client, "IRC42 Server");
	ss << c->getNick() << " " << c->getIdle();
	this->sendMessLocal("317", ss.str(), client, "seconds idle");
	this->sendMessLocal("319", c->getNick() , client, c->getChannelsList());
	this->sendMessLocal("318", c->getNick() , client, "End of WHOIS list");
	return 0;	
}

int		Server::checkHelp(Client& client, std::vector<std::string>& mess)
{
	if (mess.size() != 1)
		return (this->sendMessBot(*this->_bot[0], client, "NOTICE", "Unknown argument for !help. Usage: !help"), 0);
	this->sendMessBot(*this->_bot[0], client, "NOTICE", "Available commands: !help, !ping, !rules, !stats");
	return (0);
}

int		Server::checkPing(Client& client, std::vector<std::string>& mess)
{
	if (mess.size() != 1)
		return (this->sendMessBot(*this->_bot[0], client, "NOTICE", "Unknown argument for !ping. Usage: !ping"), 0);
	this->sendMessBot(*this->_bot[0], client, "NOTICE", "Pong!");
	return (0);
}

int		Server::checkRules(Client& client, std::vector<std::string>& mess)
{
	if (mess.size() != 1)
		return (this->sendMessBot(*this->_bot[0], client, "NOTICE", "Unknown argument for !rules. Usage: !rules"), 0);
	this->sendMessBot(*this->_bot[0], client, "NOTICE", "No insults, no flood, follow channel rules.");
	return (0);
}

int		Server::checkStats(Client& client, std::vector<std::string>& mess)
{
	std::stringstream ss;

	if (mess.size() != 1)
		return (this->sendMessBot(*this->_bot[0], client, "NOTICE", "Unknown argument for !stats. Usage: !stats"), 0);
	ss << "Current warnings: " << client.getWarn();
	this->sendMessBot(*this->_bot[0], client, "NOTICE", ss.str());
	return (0);
}

int		Server::checkNote(Client& client, std::vector<std::string>& mess)
{
	if (mess.size() != 1)
		return (this->sendMessBot(*this->_bot[0], client, "NOTICE", "Unknown argument for !note. Usage: !note"), 0);
	this->sendMessBot(*this->_bot[0], client, "NOTICE", "Met une bonne note ou jte kick tu serv");
	return (0);
}

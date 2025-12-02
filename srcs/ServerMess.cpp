#include "../includes/header.hpp"


int Server::errorState(int state, std::string const& cmd, Client& client)
{
	if (state == 0)
		return (this->sendMessLocal("461", "", client, "Password needed"), 1);
	else if (state  == 1)
		this->sendMessLocal("451", cmd, client, "You have not registered");
	return (0);
}

//:ft_irc 451 JOIN :You have not registered
void Server::sendMessLocal(std::string const& err, std::string const& cmd, Client const& c, std::string const& body)
{
	std::string err_mess;
	std::string nick;
	std::stringstream ss;

	nick = c.getNick();
	if (nick.empty())
		nick = "*";
	if (err.empty())
		ss << ":" << nick << "!" << (c.getIdent().empty() ? "*" : c.getIdent()) << "@" << c.getHost() << (cmd.empty() ? "" : (" " + cmd)) << " " << nick << (body.empty() ? "" : (" :" + body)) << "\r\n";
	else
		ss << ":irc.42.fr " << err << " " << nick << (cmd.empty() ? "" : (" " + cmd)) << (body.empty() ? "" : (" :" + body)) << "\r\n";
	err_mess = ss.str();
	send(c.getFd(), err_mess.c_str(), err_mess.size(), 0);
}

void Server::sendMessBot(Bot& b, Client const& c, std::string cmd, const std::string& mess)
{
	std::string err_mess;
	std::stringstream ss;

	ss << ":" << b.getNick() << "!"<< b.getIdent() <<"@:irc.42.fr " << cmd << " " << c.getNick() << (mess.empty() ? "" : (" :" + mess)) << "\r\n";
	err_mess = ss.str();
	send(c.getFd(), err_mess.c_str(), err_mess.size(), 0);
}

//:Jo!~jo@127.0.0.1 PRIVMSG Max :Salut Max, ça va ?
void	Server::sendMessUser(Client const& s, Client const& r, std::string const& cmd, std::string const& body)
{
	std::string err_mess;
	std::string nick;
	std::stringstream ss;


	nick = s.getNick();
	if (nick.empty())
		nick = "*";
	ss << ":" << nick << "!" << (s.getIdent().empty() ? "*" : s.getIdent()) << "@" << s.getHost() << (cmd.empty() ? "" : (" " + cmd)) << (cmd == "QUIT" ? "" : " " + r.getNick()) << (body.empty() ? "" : (" :" + body)) << "\r\n";
	err_mess = ss.str();
	send(r.getFd(), err_mess.c_str(), err_mess.size(), 0);
}

void Server::sendMessGlobal(std::string const& cmd, std::string const& mess, Client const& c)
{
	std::stringstream ss;
	std::string message;

	ss << ":" << (c.getNick().empty() ? "*" : c.getNick()) << "!~" << (c.getIdent().empty() ? "*" : c.getIdent()) << "@" << c.getHost() << " " << cmd << (mess.empty() ? "" : (" :" + mess)) << "\r\n";
	message = ss.str();
	for (size_t i = 0; i < this->_clients.size(); i++)
		send(this->_clients[i]->getFd(), message.c_str(), message.size(), 0);
	return ;
}

// Alice JOIN :#general
void Server::sendMessChannel(std::string const& channel, std::string const& argm, std::string const& mess, int sendme , Client& c)
{
	std::stringstream ss;
	std::string message;
	int i;
	std::vector<Client*> t;

	ss << ":" << (c.getNick().empty() ? "*" : c.getNick()) << "!~" << (c.getIdent().empty() ? "*" : c.getIdent()) << "@" << c.getHost() << (argm.empty() ? "" : (" " + argm))<< (mess.empty() ? "" : (" :" + mess)) << "\r\n";
	message = ss.str();
	i = this->getIndexChannel(channel);
	t = this->_channel[i]->getUsers();
	for (size_t i = 0; i < t.size(); i++)
	{
		if (sendme || t[i]->getNick() != c.getNick())
			send(t[i]->getFd(), message.c_str(), message.size(), 0);
	}
}

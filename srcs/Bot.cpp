#include "../includes/Bot.hpp"


Bot::Bot(std::string nick, std::string ident, std::string realname)
{
	this->_nick = nick;
	this->_ident = ident;
	this->_realname = realname;
	this->_messBadWords = "Warning: inappropriate language.";
	this->_messBadWords = "Warning: No spam.";
	this->_messMute = "You are muted for 5 minutes due to inappropriate language";
	this->readFile();
	return ;
}

Bot::Bot(const Bot& copy)
{
	*this = copy;
	return ;
}

Bot& Bot::operator=(const Bot& other)
{
	if (this != &other)
	{
		this->_nick = other._nick;
		this->_ident = other._ident;
		this->_realname = other._realname;
		this->_realname = other._realname;
		this->_badWords = other._badWords;
		this->_messBadWords = other._messBadWords;
		this->_messMute = other._messMute;
	}
	return (*this);
}

Bot::~Bot(void){}


/*-----------------------------------------------------------------------------------------------*/


void Bot::readFile(void)
{
	std::string buf;
	int bytesRead;
	char buff[1024];
	int fd = open("resources/badWord.txt", O_RDONLY);
	if (fd == -1)
		return ;
	while ((bytesRead = read(fd, buff, sizeof(buff))) > 0)
		buf.append(buff, bytesRead);
	close(fd);
	this->_badWords = split(buf, '\n');
	return ;
}

bool Bot::checkMessage(const std::string& message)
{
	std::vector<std::string> mess = split(message, ' ');
	for (size_t i = 0; i < mess.size(); i++)
	{
		if (std::find(this->_badWords.begin(), this->_badWords.end(), mess[i]) != this->_badWords.end())
			return (1);
	}
	return (0);
}


/*-----------------------------------------------------------------------------------------------*/


const std::string& Bot::getMessBadWords(void) const
{
	return (this->_messBadWords);
}

const std::string& Bot::getMessSpam(void) const
{
	return (this->_messBadWords);
}

const std::string& Bot::getMessMute(void) const
{
	return (this->_messMute);
}

const std::string& Bot::getNick(void) const
{
	return (this->_nick);
}

const std::string& Bot::getIdent(void) const
{
	return (this->_ident);
}


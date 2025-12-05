#ifndef BOT_HPP
# define BOT_HPP

# include "header.hpp"

class Bot
{
	private:
		Bot(void); // canonical
		Bot(const Bot& copy); // canonical
		Bot& operator=(const Bot& other); // canonical

		std::vector<std::string>	_badWords;
		std::string					_nick;
		std::string 				_ident;
		std::string 				_realname;
		std::string 				_messBadWords;
		std::string 				_messSpam;
		std::string 				_messMute;

		void				readFile(void);

	public:
		Bot(std::string nick, std::string ident, std::string realname);
		virtual ~Bot(void); // canonical

		bool				checkMessage(const std::string& message);
	
		const std::string&	getMessBadWords(void) const;
		const std::string&	getMessSpam(void) const;
		const std::string&	getMessMute(void) const;
		const std::string&	getNick(void) const;
		const std::string&	getIdent(void) const;
};

#endif

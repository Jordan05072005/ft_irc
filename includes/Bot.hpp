#ifndef BOT_HPP
# define BOT_HPP

# include "header.hpp"

class Bot {
	private:
		std::vector<std::string> _badWords;
		std::string _nick;
		std::string _ident;
		std::string _realname;
		std::string _messBadWords;
		std::string _messMute;

	public:
		Bot();
		Bot(std::string nick, std::string ident, std::string realname);
		Bot(const Bot& other);
		Bot& operator=(const Bot& other);
		~Bot();

		bool	checkMessage(const std::string& message);
		void	readFile();
		const std::string& getMessBadWords() const;
		const std::string& getMessMute() const;
		const std::string& getNick() const;
		const std::string& getIdent() const;
};

#endif
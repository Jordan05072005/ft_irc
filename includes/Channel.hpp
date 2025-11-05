#ifndef CHANNEL_HPP
# define CHANNEL_HPP

# include "header.hpp"

class Client;

typedef struct s_invite
{
	Client *client;
	std::time_t time;

}		t_invite;

typedef struct s_topic
{
	std::string topic;
	std::string modifBy;
	std::time_t time;

}		t_topic;

class Channel
{
	private :
		Channel(void); // canonical
		Channel(Channel const& copy); // canonical
		Channel&	operator=(Channel const& other); // canonical

		std::string 			_name;
		t_topic		 			_topic;
		std::vector<Client*>	_users;
		std::vector<Client*>	_operators;
		std::vector<t_invite>	_invite;

		// TODO : attribut/fonction pour Invite-only
		std::string				_channel_key;
		int						_user_limit;
	
		bool					_i; // Set/Remove Invite-only channel
		bool					_t; // Set/Remove restrict TOPIC to only operator
		bool					_k; // Set/Remove channel_key to enter
		bool					_o; //? à enlever // Give/Take channel operator privilege
		bool					_l; // Set/Remove the user limit to channel

	public :
		Channel(std::string const& name, Client const& creator);

		// à voir d'autres constructeurs si possible et si besoin
		//Channel(std::string const& name, Client const& creator, std::string const& topic); // ? est ce qu'on en a besoin, est-ce que c'est possible
		//Channel(std::string const& name, Client const& creator, std::string const& channel_key); // ? est ce qu'on en a besoin, est-ce que c'est possible

		~Channel(void); // canonical


		std::string const&				getName(void) const;
		//void							setName(Client const& by, std::string const& name); //? besoin/possibilité de rename

		t_topic const&					getTopic(void) const;
		void							setTopic(const std::string& topic, const std::string &nick);
		
		std::string const&				getChannelKey(void) const;
		//void							setChannelKey(Client const& by, std::string const& channel_key);
		
		//int const						getUserLimit(void) const;
		//void							setUserLimit(Client const& by, int nb);
		
		bool const						getOptInviteOnly(void) const;
		//void							setOptInviteOnly(Client const& by, bool opt);
		
		bool const						getOptRestrictTopic(void) const;
		//void							setOptRestrictTopic(Client const& by, bool opt);
		
		bool const						getOptChannelKey(void) const;
		//void							setOptChannelKey(Client const& by, bool opt);

		//bool const					getOptOperatorPrivilege(void) const;
		//void							setOptOperatorPrivilege(Client const& by, bool opt);
		
		//bool const					getOptUserLimit(void) const;
		//void							setOptUserLimit(Client const& by, bool opt);

		std::vector<Client*> const&		getUsers(void) const;
		std::vector<Client*> const&		getOperators(void) const;
		const std::vector<t_invite>&	getInvite(void) const;


		void							addUser(Client const& user);
		// TODO : voir si possibilité de rajouter operateur autrement que dans la creation
		void							addOperator(Client const& operator);
		void							addInvite(Client& client);

		void							removeUser(std::string& nick);
		void							removeInvite(void);

		int								checkUser(const std::string& nick);
		int								checkOperator(const std::string& nick);
		int								checkInvite(const std::string& nick);

		std::string						createStringUsers(void) const;
};

#endif

#ifndef CHANNEL_HPP
# define CHANNEL_HPP

# include "header.hpp"

class Client;

typedef struct s_invitee{
	Client *client;
	std::time_t time;
}		t_invitee;

typedef struct s_topic{
	std::string topic;
	std::string modifBy;
	std::time_t time;
}		t_topic;

class Channel
{
	private :

		std::string 		name;
		t_topic		topic;
		std::vector<Client>	users;
		std::vector<Client>	operators;
		std::vector<t_invitee>	_invitee;

		// TODO : attribut/fonction pour Invite-only
		std::string			channel_key;
		int					user_limit;
	
		bool				i; // Set/Remove Invite-only channel
		bool				t; // Set/Remove restrict TOPIC to only operator
		bool				k; // Set/Remove channel_key to enter
		bool				o; //? à enlever // Give/Take channel operator privilege
		bool				l; // Set/Remove the user limit to channel

	public :
		Channel(void); // canonical
		Channel(std::string const& name, Client const& creator);
		Channel(Channel const& copy); // canonical
		Channel&	operator=(Channel const& other); // canonical
		// // à voir d'autres constructeurs si possible et si besoin
		// Channel(std::string const& name, Client const& creator, std::string const& topic); // ? est ce qu'on en a besoin, est-ce que c'est possible
		// Channel(std::string const& name, Client const& creator, std::string const& channel_key); // ? est ce qu'on en a besoin, est-ce que c'est possible

		~Channel(void); // canonical


		// // des getteurs et setteurs au cas où
		std::string const&			getName(void) const;
		// void						setName(Client const& by, std::string const& name); //? besoin/possibilité de rename

		const t_topic&	getTopic(void) const;
		void						setTopic(const std::string & topic, const std::string &nick);
		
		// std::string const&			getChannelKey(void) const;
		// void						setChannelKey(Client const& by, std::string const& channel_key);
		
		// int const					getUserLimit(void) const;
		// void						setUserLimit(Client const& by, int nb);
		
		bool					getOptInviteOnly(void) const;
		// void						setOptInviteOnly(Client const& by, bool opt);
		
		bool					getOptRestrictTopic(void) const;
		// void						setOptRestrictTopic(Client const& by, bool opt);
		
		// bool const					getOptOperatorPrivilege(void) const;
		// void						setOptOperatorPrivilege(Client const& by, bool opt);

		
		// bool const					getOptUserLimit(void) const;
		// void						setOptUserLimit(Client const& by, bool opt);

		const std::vector<Client>&	getUsers(void) const;
		const std::vector<Client>&	getOperators(void) const;


		void						addUsers(Client const& by, Client const& user);
		void						addOperator(Client const& by, Client const& operato);
		const std::vector<t_invitee>&	getInvitee()const;
		void						addInvitee(Client& client);
		int							checkUser(const std::string& nick);
		int							checkOperator(const std::string& nick);
		int							checkInvitee(const std::string& nick);
		void						delInvitee();
		void						delUsers(std::string& nick);
};

#endif

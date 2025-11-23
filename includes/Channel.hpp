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

		std::time_t				_creationtime;
		std::string 			_name;
		t_topic		 			_topic;
		std::vector<Client*>	_users;
		std::vector<Client*>	_operators;
		std::vector<t_invite>	_invite;

		std::string				_channel_key;
		size_t					_user_limit;
	
		bool					_i; // Set/Remove Invite-only channel
		bool					_t; // Set/Remove restrict TOPIC to only operator
		bool					_k; // Set/Remove channel_key to enter
		bool					_l; // Set/Remove the user limit to channel

	public :

		Channel(void); // canonical
		Channel(std::string const& name, Client* creator);
		Channel(std::string const& name, std::string const& key, Client* creator);
		Channel(Channel const& copy); // canonical
		Channel&	operator=(Channel const& other); // canonical
		~Channel(void); // canonical


		std::time_t const&				getCreationTime(void) const;
		std::string const&				getName(void) const;
		//void							setName(Client const& by, std::string const& name); //? besoin/possibilité de rename

		t_topic const&					getTopic(void) const;
		void							setTopic(const std::string& topic, const std::string& nick);
		
		std::string const&				getChannelKey(void) const;
		void							setChannelKey(std::string const& channel_key);
		
		size_t							getUserLimit(void) const;
		void							setUserLimit(size_t nb);
		
		bool							getOptInviteOnly(void) const;
		void							setOptInviteOnly(bool opt);
		
		bool							getOptRestrictTopic(void) const;
		void							setOptRestrictTopic(bool opt);
		
		bool							getOptChannelKey(void) const;
		void							setOptChannelKey(bool opt);

		bool							getOptUserLimit(void) const;
		void							setOptUserLimit(bool opt);

		std::vector<Client*> const&		getUsers(void) const;
		std::string						getUsersCountStr(void) const;
		size_t							getUsersCountNb(void) const;
		std::vector<Client*> const&		getOperators(void) const;
		const std::vector<t_invite>&	getInvite(void) const;


		void							addUser(Client& user);
		void							addOperator(Client& user);
		void							addInvite(Client& client);

		void							removeUser(std::string const& nick);
		void							removeInvite(void);
		void							removeOperator(std::string const& nick);

		void							delInvite(Client& client);

		int								checkUser(const std::string& nick) const;
		int								checkOperator(const std::string& nick) const;
		int								checkInvite(const std::string& nick) const;

		std::string						createStringUsers(void) const;
		std::string						createStringModes(void) const;
};

#endif

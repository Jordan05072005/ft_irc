#ifndef Client_HPP
# define Client_HPP

# include "header.hpp"

class Client
{
	private:
		Client(void);

		int _fd;
		std::string _name;
		sockaddr_in _addr;
		socklen_t _len;
		std::string _buff;

	public:
		Client(int fd, sockaddr_in addr, socklen_t len);
		Client(const Client& cpy);
		Client& operator=(const Client& cpy);
		~Client(void);

		int getfd(void);
		void setfd(int fd);
		std::string& getbuf(void);
		void setbuf(char *buf, int oct);
		std::string& getnick();
		void setnick(char *nick);
		bool getlogin();
		void setlogin(bool e);
};

#endif

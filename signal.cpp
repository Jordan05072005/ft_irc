#include "includes/header.hpp"

void handle_signal(int sig)
{
	Server& serv = Server::getInstance();
	if (sig == SIGINT || sig == SIGTERM)
	{
			serv.closeAll();
			std::cout << "bye" << std::endl;
	}
	return ;
}

void setup_signals(void)
{
	struct sigaction sa;
	sa.sa_handler = handle_signal;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;

	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);
	signal(SIGPIPE, SIG_IGN);
	return ;
}

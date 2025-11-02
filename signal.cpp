#include "includes/header.hpp"

void handle_signal(int sig) {
	Server& serv = Server::getInstance();
	if (sig == SIGINT || sig == SIGTERM){
			serv.closeAll();
			std::cout << "bye" << std::endl;
			exit(0);
	}
}

void setup_signals() {
	struct sigaction sa;
	sa.sa_handler = handle_signal;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;

	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);
	signal(SIGPIPE, SIG_IGN);
}

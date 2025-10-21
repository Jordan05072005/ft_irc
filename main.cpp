#include "includes/ft_irc.hpp"

#include <stdlib.h>


int main(int ac, char **av){
	if (ac != 3)
		return 1;
	try{
		ft_irc serv = ft_irc(atoi(av[1])); // changer atoi par ft_atoi
		serv.startSev();
	}catch(std::exception &e){
		std::cerr << e.what() << std::endl;
	}

}
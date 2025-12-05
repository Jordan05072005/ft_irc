/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jguaglio <guaglio.jordan@gmail.com>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/21 13:48:35 by jguaglio          #+#    #+#             */
/*   Updated: 2025/10/21 13:48:35 by jguaglio         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/header.hpp"


/*

	Test command for fragmented data sent :

			socat <option> flux1<,mode1,mode2,...> flux2<,mode1,mode2,...>

			socat -d -d STDIN,raw,echo=1 tcp:127.0.0.1:6687


		-> -d -d = print double debug, double for more verbose

		-> STDIN = input flux
		-> mode raw = instant sending, consequence : no signal intercepted
		-> mode echo=1 = write on the input terminal what we've sent

		-> tcp:127.0.0.1:6687 = open connection tcp socket on local address and port 6687

*/


int	parsing(std::string port)
{
	for (size_t i = 0; i < port.length(); ++i)
	{
		if (std::isdigit(port[i]) == false)
			return (1);
	}
	return (0);
}

int main(int ac, char **av)
{
	if (ac != 3)
	{
		std::cout << "Usage : <port> <password>" << std::endl;
		return (1);
	}

	std::string port = av[1];
	std::string password = av[2];
	if (parsing(port) == 1)
	{
		std::cerr << "Error : invalid port" << std::endl;
		return (1);
	}
	try
	{
		Server& serv = Server::getInstance();
		serv.init(std::atoi(av[1]), password);
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}

	return (0);
}

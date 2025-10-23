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

#include "includes/header.hpp"

int	parsing(std::string port, std::string password)
{
	for (int i = 0; i < port.length(); ++i)
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
	switch (parsing(port, password)) // * vérifier si sécurité mot de passe à gérer
	{
		case 1 :
		{
			std::cerr << "Error : invalid port" << std::endl;
			return (1);
		}

		case 2 :
		{
			std::cerr << "Error : invalid password" << std::endl;
			return (1);
		}

		default :
			break ;
	}

	try
	{
		Server serv(std::atoi(av[1]), password);
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}

	return (0);
}

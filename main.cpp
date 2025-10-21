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

#include "includes/ft_irc.hpp"


int main(int ac, char **av){
	if (ac != 3)
		return 1;
	try{
		ft_irc serv = ft_irc(atoi(av[1]), av[2]);
		serv.startSev();
	}catch(std::exception &e){
		std::cerr << e.what() << std::endl;
	}

}
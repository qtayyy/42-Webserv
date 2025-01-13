/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qtay <qtay@student.42kl.edu.my>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/12 16:34:35 by qtay              #+#    #+#             */
/*   Updated: 2025/01/12 17:00:50 by qtay             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Webserv.hpp"
#include "Cluster.hpp"

int	main(int argc, char **argv)
{
	std::string	defaultPath = "somedefaultpath";
	if (argc > 2)
	{
		std::cerr << RED "Usage error: invalid num of args\n" <<
		"Usage: ./webserv [config file]\n" RESET;
		return (EXIT_FAILURE);
	}
	if (argc == 2)
		defaultPath = argv[1];
	Cluster	cluster(defaultPath);
}

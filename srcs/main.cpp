/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qtay <qtay@student.42kl.edu.my>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/12 16:34:35 by qtay              #+#    #+#             */
/*   Updated: 2025/01/21 18:03:31 by qtay             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Webserv.hpp"
#include "Cluster.hpp"
#include "Log.hpp"

int	main(int argc, char **argv)
{

	// Logger logger;
	// Log::log << Logger::setStream("hi.txt", std::ios::app) << "hello"  << std::endl;
	// logger  << "hi"  << std::endl;

	std::string	defaultPath = "conf/good/custom.conf";
	if (argc > 2)
	{
		std::cerr << RED "Usage error: invalid num o f args\n" <<
		"Usage: ./webserv [config file]\n" RESET;
		return (EXIT_FAILURE);
	}
	if (argc == 2)
		defaultPath = argv[1];
	try
	{
		Cluster	cluster(defaultPath);
		cluster.parse();
		cluster.init();
		cluster.run();
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
}

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cluster.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qtay <qtay@student.42kl.edu.my>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/12 17:06:29 by qtay              #+#    #+#             */
/*   Updated: 2025/01/15 15:29:20 by qtay             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Cluster.hpp"

std::string	strTrim(std::string str, std::string delims)
{
	size_t start = str.find_first_not_of(delims);
	if (start != std::string::npos)
		str = str.substr(start);
	else
		str = "";
    size_t end = str.find_last_not_of(delims);
	if (end != std::string::npos)
		str = str.substr(0, end + 1);
	else
		str = "";
	return (str);
}

void	Cluster::parse(void)
{
	std::vector<std::string>	tokens;
	tokens = tokenizeConfig(this->_configPath);
	try
	{
		parseConfig(tokens);
		std::cout << GREEN "Config file parsing complete\n" RESET;
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
}

std::vector<std::string>	Cluster::tokenizeConfig(std::string &configPath)
{
	std::ifstream configFile(configPath);

	if (!configFile.is_open())
		throw ConfigFileException( RED + configPath + ": No such file or directory" RESET);
	std::string	line;
	std::vector<std::string>	tokens;
	while (std::getline(configFile, line))
	{
		line = strTrim(line, " \t");
		if (!line.empty() && line[0] == '#')
			continue ;
		size_t	start = 0, end;
		while (start < line.size())
		{
			if ((end = line.find_first_of(" \t;", start)) == std::string::npos)
			{
				tokens.push_back(line.substr(start));
				break ;
			}
			if (end > start)
				tokens.push_back(line.substr(start, end - start));
			if (line[end] == ';')
				tokens.push_back(";");
			start = end + 1;
		}
	}
	configFile.close();
	return (tokens);
}

void	Cluster::parseConfig(std::vector<std::string> tokens)
{
	for (int i = 0; i < tokens.size(); i++)
	{
		if (tokens[i++] == "server")
		{
			ServerBlock				server;
			if ((i = server.parseServer(tokens, i)) < 0)
				throw ConfigFileException(RED + _configPath + " error" RESET);
			this->_servers.push_back(server);
		}
		else
			throw ConfigFileException(RED + _configPath + " error: unknown directive: " + tokens[i] + RESET);
	}
	
}
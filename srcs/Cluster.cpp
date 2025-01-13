/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cluster.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qtay <qtay@student.42kl.edu.my>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/12 17:06:29 by qtay              #+#    #+#             */
/*   Updated: 2025/01/12 20:15:44 by qtay             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Cluster.hpp"

Cluster::Cluster(std::string &configPath)
{
	std::vector<std::string>	tokens;
	tokens = Cluster::tokenizeConfig(configPath); // tokenize
	Cluster::parseConfig(tokens);
	// pass to server block (parsing)
}

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

std::vector<std::string>	Cluster::tokenizeConfig(std::string &configPath)
{
	std::ifstream configFile(configPath);

	if (!configFile.is_open())
		throw ConfigFileException( RED + configPath + ": No such file or directory" RESET);
	std::string	line;
	std::vector<std::string>	tokens;
	while (std::getline(configFile, line))
	{
		line = strTrim(line, " \t;");
		if (!line.empty() && line[0] == '#')
			continue ;
		std::stringstream	ss(line);
		std::string			token;
		while (std::getline(ss, token, ' '))
		{
			if (!token.empty())
				tokens.push_back(token);
		}
	}
	configFile.close();
	return (tokens);
}

void	Cluster::parseConfig(std::vector<std::string> tokens)
{
	// if (tokens.size() < 3 || tokens[0] != "server" || tokens[1] != "{")
	// 	throw ConfigFileException(RED "config file error: invalid config syntax\n" RESET);

	// ServerBlock				server;
	// std::string				directive;
	// std::vector<std::string> args;
	for (size_t i = 0; i < tokens.size(); i++)
	{
		if (tokens.size() > 2 && tokens[i] == "server")
	}	
}
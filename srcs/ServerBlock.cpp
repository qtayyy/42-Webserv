/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerBlock.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qtay <qtay@student.42kl.edu.my>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/11 21:53:55 by qtay              #+#    #+#             */
/*   Updated: 2025/01/22 18:22:53 by qtay             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerBlock.hpp"

// ============================= PARSE CONFIG FILE ============================

/**
 * @brief	Sets up mandatory values in a server block if they are not
 * 			specified by the config file.
 */
void	ServerBlock::initDefaultServerBlockConfig(void)
{
	if (_root == "")
		setRoot();
	if (_index.empty())
		setIndex();
	if (_listen.empty())
		setListen();
	if (_limitExcept.empty())
		setLimitExcept();
	if (_autoindex == -1)
		setAutoindex();
	if (_clientMaxBodySize == -1)
		setClientMaxBodySize();
	// if (_errorPage.empty()) // Change to dynamically generate
	// 	setErrorPage(); // change
}

/**
 * @brief	Parses all server blocks in the config file.
 * @return	Returns the index of the last parsed token or -1 if an error is
 * 			found.
 */
int	ServerBlock::parseServer(std::vector<std::string> tokens, int i)
{
	bool		isDirective = true;
	std::string	directive;
	std::vector<std::string> args;

	if (tokens[i++] != "{")
		return (-1);
	for (; i < (int)(tokens.size() - 1); i++)
	{
		if (tokens[i] == "}")
			break ;
		if (tokens[i] == ";")
			isDirective = true;
		else if (serverParseMap.find(tokens[i]) != serverParseMap.end() && isDirective) // is directive
		{
			if (directive != "")
			{
				(this->*ServerBlock::serverParseMap[directive])(args);
				args.clear(); directive.clear();
			}
			directive = tokens[i];
			isDirective = false;
		}
		else if (tokens[i] == "location" && isDirective)
		{
			if (directive != "") // hmmm a bit ugly
			{
				(this->*ServerBlock::serverParseMap[directive])(args);
				args.clear(); directive.clear();
			}
			LocationBlock	location(this);
			 if ((i = location.parseLocation(tokens, i)) < 0)
				return (i);
			this->_location.push_back(location);
			isDirective = true;
		}
		else if (!isDirective)
			args.push_back(tokens[i]);
		else
		{
			std::cerr << RED"config error: unknown directive: '" << tokens[i] << "'\n" << RESET;
			return (-1);
		}
	}
	if (directive != "") // hmmm a bit ugly
	{
		(this->*ServerBlock::serverParseMap[directive])(args);
		args.clear(); directive.clear();
	}
	if (tokens[i] != "}")
		return (-1);
	initDefaultServerBlockConfig();
	for (size_t i = 0; i < _location.size(); i++)
		_location[i].initDefaultLocationBlockConfig();
	printBlock();
	for (size_t i = 0; i < _location.size(); i++)
		_location[i].printBlock();
	return (i);
}

// ================================= SETTERS =================================

// HELPER FUNCTION FOR SETLISTEN()
uint32_t ServerBlock::ipToInt(const std::string &ip)
{
	struct in_addr	addr;

	int	result = inet_pton(AF_INET, ip.c_str(), &addr);
	if (result == 1)
		return (ntohl(addr.s_addr));
	else
		return (0); // 0.0.0.0 will also return 0
}

/**
 * @brief	Set the ports and optionally, the IP addresses/network interfaces the
 * 			server should listen on.
 * SYNTAX:	listen [ip:port]
 */
void	ServerBlock::setListen(std::vector<std::string> args)
{
	std::string	ipStr = "0.0.0.0";
	std::string	portStr = "8080";

	if (args.empty())
	{
		_listen.clear();
		_listen.push_back(std::make_pair(ipToInt(ipStr), std::atoi(portStr.c_str())));
		return ;
	}

	uint32_t	ip;
	int			port;
	size_t	colonPos;
	if (args.size() != 1)
		std::cerr << RED "listen error: invalid num of args! Set to first IP:Port\n" RESET;
	colonPos = args[0].find(':');
	if (colonPos != std::string::npos)
	{
		ipStr = args[0].substr(0, colonPos);
		portStr = args[0].substr(colonPos + 1);
	}
	else
		portStr = args[0];
	ip = ipToInt(ipStr);
	port = std::atoi(portStr.c_str());
	if ((ip == 0 && ipStr != "0.0.0.0") || port > 65535 || port < 1024)
	{
		std::cerr << RED "listen error: invalid IP or port\n" RESET;
		return ;
	}
	std::pair<uint32_t, int> newListen = std::make_pair(ip, port);
	if (std::find(_listen.begin(), _listen.end(), newListen) != _listen.end())
	{
		std::cerr << RED "listen error: duplicated port\n" RESET;
		return ;
	}
	_listen.push_back(newListen);
}

/**
 * @brief	Set up server name(s) that correspond to a particular ip:port.
 * SYNTAX:	server_name [name 1] [name 2] ...
 */
void	ServerBlock::setServerName(std::vector<std::string> args)
{
	if (args.size() < 1)
	{
		std::cerr << RED "server_name error: invalid num of args\n" RESET;
		return ;
	}
	for (size_t i = 0; i < args.size(); i++)
	{
		if (std::find(_serverName.begin(), _serverName.end(), args[i]) != _serverName.end())
		{
			std::cerr << RED "server_name error: duplicated server name\n" RESET;
			return ;
		}
		_serverName.push_back(args[i]);
	}
}

// ================================== UTILS ==================================

/**
 * @brief	Prints all directives and their values in the server block.
 */
void	ServerBlock::printBlock()
{
	std::cout << "root: " << this->getRoot() << "\n";
	std::cout << "autoindex: " << this->getAutoindex() << "\n";
	std::cout << "client max body size: " << this->getClientMaxBodySize() << "\n";
	std::vector<std::string> allowedMethods = this->getLimitExcept();
	for (std::vector<std::string>::iterator it = allowedMethods.begin(); it != allowedMethods.end(); it++)
		std::cout << "allowed methods: "  << *it << std::endl;
	std::vector<std::string> allIndexes = this->getIndex();
	for (std::vector<std::string>::iterator it = allIndexes.begin(); it != allIndexes.end(); it++)
		std::cout << "indexes: "  << *it << std::endl;
	std::map<std::string, std::string> allCGIs = this->getCgiScript();
	for (std::map<std::string, std::string>::iterator it = allCGIs.begin(); it != allCGIs.end(); it++)
		std::cout << "cgi extension: "  << it->first << "; cgi path: " << it->second << std::endl;
	std::vector<std::pair<uint32_t, int> > allListens = this->getListen();
	for (std::vector<std::pair<uint32_t, int> >::iterator it = allListens.begin(); it != allListens.end(); it++)
		std::cout << "ip: "  << intToIp(it->first) << "; port: " << it->second << std::endl;
	std::vector<std::string> servernames = this->getServerName();
	for (std::vector<std::string>::iterator it = servernames.begin(); it != servernames.end(); it++)
		std::cout << "server names: "  << *it << std::endl;
	std::map<int, std::string> errorpages = this->getErrorPage();
	for (std::map<int, std::string>::iterator it = errorpages.begin(); it != errorpages.end(); it++)
		std::cout << "error code: " << it->first << " " << it->second << std::endl;
	std::cout << "\n";
}

// ================================= STATIC =================================

std::map<std::string, void (ServerBlock::*)(std::vector<std::string>)>
	ServerBlock::initServerMap(void)
{
	std::map<std::string, void (ServerBlock::*)(std::vector<std::string>)>	serverMap;

	serverMap["listen"] = &ServerBlock::setListen;
	serverMap["root"] = &ServerBlock::setRoot;
	serverMap["index"] = &ServerBlock::setIndex;
	serverMap["error_page"] = &ServerBlock::setErrorPage;
	serverMap["client_max_body_size"] = &ServerBlock::setClientMaxBodySize;
	serverMap["server_name"] =  &ServerBlock::setServerName;
	serverMap["cgi_script"] = &ServerBlock::setCgiScript;
	serverMap["limit_except"] = &ServerBlock::setLimitExcept;
	serverMap["autoindex"] = &ServerBlock::setAutoindex;
	// serverMap["location"] = &ServerBlock::setLocation;

	return (serverMap);
}

std::map<std::string, void (ServerBlock::*)(std::vector<std::string>)> ServerBlock::serverParseMap = initServerMap();

// int	main()
// 	ServerBlock test;
// 	std::vector<std::string> maxsize = {"240000"};
// 	std::vector<std::string> root = {"/newroot/haha"};//, "/another/one"};
// 	std::vector<std::string> autoindex = {"on"};
// 	std::vector<std::string> methods = {"GET", "DELETE"};
// 	std::vector<std::string> indexes = {"newindex.html", "newnew.htm"};
// 	std::vector<std::string> newcgi = {".cgi", "/mycgipath"};
// 	std::vector<std::string> listen1 = {"3434"};
// 	// std::vector<std::string> listen2 = {"13434"};
// 	std::vector<std::string> servernames = {"example.com", "example2.com"};

// 	std::cout << "[ BEFORE ]\n";
// 	test.printBlock();
// 	try
// 	{
// 		test.setRoot(root); 
// 		test.setAutoindex(autoindex);
// 		test.setClientMaxBodySize(maxsize);
// 		test.setLimitExcept(methods);
// 		test.setIndex(indexes);
// 		test.setCgiScript(newcgi);
// 		test.setListen(listen1);
// 		// test.setListen(listen2);
// 		test.setServerName(servernames);
// 	}
// 	catch(const std::exception& e)
// 	{
// 		std::cerr << e.what() << '\n';
// 	}
// 	std::cout << "\n[ AFTER ]\n";
// 	test.printBlock();
// }

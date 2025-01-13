/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationBlock.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qtay <qtay@student.42kl.edu.my>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/12 15:21:32 by qtay              #+#    #+#             */
/*   Updated: 2025/01/12 20:05:00 by qtay             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "LocationBlock.hpp"

// ================================= SETTERS =================================

/**
 * @brief	The URI that triggers the use of this specific location block.
 */
void	LocationBlock::setUri(std::vector<std::string> args)
{
	if (args.size() != 1)
	{
		std::cerr << RED "location error: uri: invalid num of args\n" RESET; 
		return ;
	}
	this->_uri = args[0];
}

/**
 * @brief	Used to map the given location URI path (see above) to a filesystem
 * 			directory.
 * SYNTAX:	alias [actual filesystem directory]
 */
void	LocationBlock::setAlias(std::vector<std::string> args)
{
	if (args.size() != 1)
	{
		std::cerr << RED "location error: alias: invalid num of args\n" RESET; 
		return ;
	}
	this->_alias = args[0];
}

/**
 * @brief	Redirects the given URI to another.
 * SYNTAX:	return [status code] [new URI]
 */
void	LocationBlock::setReturn(std::vector<std::string> args)
{
	if (args.size() != 2)
	{
		std::cerr << RED "location error: return: invalid num of args\n" RESET; 
		return ;
	}
	int	statusCode;
	statusCode = std::atoi(args[0].c_str());
	if (statusCode >= 300 && statusCode <= 399)
		this->_return = std::make_pair(statusCode, args[1]);
	else
		std::cerr << RED "location error: return: invalid status code\n" RESET; 
}

// ================================= STATIC ==================================

std::map<std::string, void (LocationBlock::*)(std::vector<std::string>)>	LocationBlock::initLocationMap(void)
{
	std::map<std::string, void (LocationBlock::*)(std::vector<std::string>)>	locationMap;

	locationMap["location"] = &LocationBlock::setUri; // KIV
	locationMap["alias"] = &LocationBlock::setAlias;
	locationMap["autoindex"] = &LocationBlock::setAutoindex;
	locationMap["cgi_script"] = &LocationBlock::setCgiScript;
	locationMap["client_max_body_size"] = &LocationBlock::setClientMaxBodySize;
	locationMap["error_page"] = &LocationBlock::setErrorPage;
	locationMap["index"] =  &LocationBlock::setIndex;
	locationMap["limit_except"] = &LocationBlock::setLimitExcept;
	locationMap["return"] = &LocationBlock::setReturn;
	locationMap["root"] = &LocationBlock::setRoot;

	return (locationMap);
}

std::map<std::string, void (LocationBlock::*)(std::vector<std::string>)>	LocationBlock::locationParseMap = initLocationMap();

// ================================= UTILS ==================================

void	LocationBlock::printBlock()
{
	std::cout << "root: " << this->getRoot() << "\n";
	std::cout << "autoindex: " << std::boolalpha << this->getAutoindex() << "\n";
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
	std::cout << "uri: " << this->getUri() << "\n";
	std::cout << "alias: " << this->getAlias() << "\n";
	std::cout << "return: " << this->getReturn().first << " " << this->getReturn().second << "\n";
}

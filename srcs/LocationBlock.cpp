/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationBlock.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qtay <qtay@student.42kl.edu.my>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/12 15:21:32 by qtay              #+#    #+#             */
/*   Updated: 2025/01/20 13:35:33 by qtay             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "LocationBlock.hpp"

// ============================= PARSE CONFIG FILE ============================

/**
 * @brief	Initializes attributes that are not overriden by the location
 * 			block itself. Values are taken from the parent server.
 */
void	LocationBlock::initDefaultLocationBlockConfig(void)
{
	if (_root == "")
		_root = _parentServerBlock->getRoot();
	if (_autoindex == -1)
		_autoindex = _parentServerBlock->getAutoindex();
	if (_index.empty())
		_index = _parentServerBlock->getIndex(); // Deep copy..?
	if (_errorPage.empty())
		_errorPage = _parentServerBlock->getErrorPage();
	if (_limitExcept.empty())
		_limitExcept = _parentServerBlock->getLimitExcept();
	if (_cgiScript.empty())
		_cgiScript = _parentServerBlock->getCgiScript();
	if (_clientMaxBodySize == 0)
		_clientMaxBodySize = _parentServerBlock->getClientMaxBodySize();
}

/**
 * @brief	Parses directives and arguments in a location block.
 * @return	Returns the index of the last parsed token or -1 if an error is
 * 			found.
 */
int	LocationBlock::parseLocation(std::vector<std::string> tokens, int i)
{
	bool		isDirective = true;
	std::string	directive;
	std::vector<std::string> args;

	if (i + 2 < tokens.size())
	{
		this->_uri = tokens[++i];
		if (tokens[++i] != "{")
			return (-1);
		i++;
	}
	for (i; i < tokens.size() - 1; i++)
	{
		if (tokens[i] == "}")
			break ;
		if (tokens[i] == ";")
			isDirective = true ;
		else if (locationParseMap.find(tokens[i]) != locationParseMap.end() && isDirective)
		{
			if (directive != "")
			{
				(this->*LocationBlock::locationParseMap[directive])(args);
				args.clear(); directive.clear();
			}
			directive = tokens[i];
			isDirective = false;
		}
		else if (!isDirective)
			args.push_back(tokens[i]);
		else
		{
			std::cerr << RED"config error: unknown directive: " << tokens[i] << "\n" << RESET;
			return (-1);
		}
	}
	if (directive != "")
	{
		(this->*LocationBlock::locationParseMap[directive])(args);
		args.clear(); directive.clear();
	}
	if (tokens[i] != "}")
		return (-1);
	return (i);
}

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

	// locationMap["location"] = &LocationBlock::setUri; // KIV
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
	std::cout << "location root: " << this->getRoot() << "\n";
	std::cout << "location autoindex: " << this->getAutoindex() << "\n";
	std::cout << "location client max body size: " << this->getClientMaxBodySize() << "\n";
	std::vector<std::string> allowedMethods = this->getLimitExcept();
	for (std::vector<std::string>::iterator it = allowedMethods.begin(); it != allowedMethods.end(); it++)
		std::cout << "location allowed methods: "  << *it << std::endl;
	std::vector<std::string> allIndexes = this->getIndex();
	for (std::vector<std::string>::iterator it = allIndexes.begin(); it != allIndexes.end(); it++)
		std::cout << "location indexes: "  << *it << std::endl;
	std::map<std::string, std::string> allCGIs = this->getCgiScript();
	for (std::map<std::string, std::string>::iterator it = allCGIs.begin(); it != allCGIs.end(); it++)
		std::cout << "location cgi extension: "  << it->first << "; cgi path: " << it->second << std::endl;
	std::cout << "location uri: " << this->getUri() << "\n";
	std::cout << "location alias: " << this->getAlias() << "\n";
	if (_return.first != 0)
		std::cout << "location return: " << this->getReturn().first << " " << this->getReturn().second << "\n";
	std::cout << "\n";
}

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationBlock.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qtay <qtay@student.42kl.edu.my>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/12 15:21:32 by qtay              #+#    #+#             */
/*   Updated: 2025/01/22 18:33:13 by qtay             ###   ########.fr       */
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
	// if (_errorPage.empty()) // Change this part so that all error pages have a default
	// 	_errorPage = _parentServerBlock->getErrorPage();
	if (_limitExcept.empty())
		_limitExcept = _parentServerBlock->getLimitExcept();
	// if (_cgiScript.empty())
	// 	_cgiScript = _parentServerBlock->getCgiScript();
	if (_clientMaxBodySize == -1)
		_clientMaxBodySize = _parentServerBlock->getClientMaxBodySize();
	inheritErrorPages(_parentServerBlock->getErrorPage());
	// inheritCgiScripts(_parentServerBlock->getCgiScript());
}

void	LocationBlock::inheritErrorPages(std::map<int, std::string> parentErrorPages)
{
	for (std::map<int, std::string>::iterator it = parentErrorPages.begin(); it != parentErrorPages.end(); it++)
	{
		if (this->_errorPage.find(it->first) == this->_errorPage.end())
			this->_errorPage[it->first] = it->second;
	}
}

// void	LocationBlock::inheritCgiPass(std::string parentCgiPass)
// {
// 	for (std::map<std::string, std::string>::iterator it = parentCgiScripts.begin(); it != parentCgiScripts.end(); it++)
// 	{
// 		if (this->_cgiScript.find(it->first) == this->_cgiScript.end())
// 			this->_cgiScript[it->first] = it->second;
// 	}
// }

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

	if (i + 2 < (int)tokens.size())
	{
		this->_uri = tokens[++i];
		if (tokens[++i] != "{")
		{
			std::cerr << RED "Missing location uri: " RESET;
			return (-1);
		}
		i++;
	}
	for (; i < (int)(tokens.size() - 1); i++)
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
			std::cerr << RED"config error: unknown directive: '" << tokens[i] << "'" << "\n" RESET;
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

/**
 * @brief	Sets up CGI to handle non-htm/html files.
 * SYNTAX:	cgi_pass [path_to_binary]
 */
void	LocationBlock::setCgiPass(std::vector<std::string> args)
{
	if (args.size() != 1)
	{
		if (!args.empty())
			std::cerr << RED "cgi_pass error: invalid num of args.\n" RESET;
		return ;
	}
	this->_cgiPass = args[0];
}

void	LocationBlock::setUploadPath(std::vector<std::string> args)
{
	if (args.size() != 1)
	{
		if (!args.empty())
			std::cerr << RED "upload_path error: invalid num of args. Upload path is set to default.\n" RESET;
		return ;
	}
	this->_uploadPath = args[0];
}

// ================================= STATIC ==================================

std::map<std::string, void (LocationBlock::*)(std::vector<std::string>)>	LocationBlock::initLocationMap(void)
{
	std::map<std::string, void (LocationBlock::*)(std::vector<std::string>)>	locationMap;

	// locationMap["location"] = &LocationBlock::setUri; // KIV
	locationMap["alias"] = &LocationBlock::setAlias;
	locationMap["autoindex"] = &LocationBlock::setAutoindex;
	locationMap["cgi_pass"] = &LocationBlock::setCgiPass;
	locationMap["client_max_body_size"] = &LocationBlock::setClientMaxBodySize;
	locationMap["error_page"] = &LocationBlock::setErrorPage;
	locationMap["index"] =  &LocationBlock::setIndex;
	locationMap["limit_except"] = &LocationBlock::setLimitExcept;
	locationMap["return"] = &LocationBlock::setReturn;
	locationMap["root"] = &LocationBlock::setRoot;
	locationMap["upload_path"] = &LocationBlock::setUploadPath;

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
	std::cout << "location cgi_pass: " << this->getCgiPass() << "\n";
	std::cout << "location upload_path: " << this->getUploadPath() << "\n";
	std::cout << "location uri: " << this->getUri() << "\n";
	std::cout << "location alias: " << this->getAlias() << "\n";
	if (_return.first != 0)
		std::cout << "location return: " << this->getReturn().first << " " << this->getReturn().second << "\n";
	std::map<int, std::string> errorpages = this->getErrorPage();
	for (std::map<int, std::string>::iterator it = errorpages.begin(); it != errorpages.end(); it++)
		std::cout << "location error code: " << it->first << " " << it->second << std::endl;
	std::cout << "\n";
}

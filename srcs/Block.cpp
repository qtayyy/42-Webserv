/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Block.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qtay <qtay@student.42kl.edu.my>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/10 14:05:46 by qtay              #+#    #+#             */
/*   Updated: 2025/01/22 18:24:16 by qtay             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Block.hpp"

// ================================= SETTERS =================================


/**
 * @brief	Sets up default or custom error pages.
 * SYNTAX:	error_page [error code 1] [error code 2] ... [path to error page]
 * Discuss if we wanna generate error pages dynamically for the default
 */
void			Block::setErrorPage(std::vector<std::string> args)
{
	if (args.size() < 2)
	{
		// initDefaultErrorPage();
		if (!args.empty())
			std::cerr << RED "error_page error: invalid num of args. Resetting to default error pages...\n" RESET;
		return ;
	}

	int			errorCode;
	std::string	errorPagePath = args.back();
	for (size_t i = 0; i < args.size() - 1; i++)
	{
		errorCode = std::atoi(args.at(i).c_str());
		if (errorCode < 400 || errorCode > 599)
		{
			// initDefaultErrorPage(); // generate dynamically
			std::cerr << RED "error_page error: invalid error code.\n" RESET;
			continue ;
		}
		this->_errorPage[errorCode] = errorPagePath;
	}
}

// void	Block::initDefaultErrorPage(void)
// {
// 	this->_errorPage[301] = "/var/www/error301.html";
// 	this->_errorPage[404] = "/var/www/error404.html";
// 	this->_errorPage[405] = "/var/www/error405.html";
// 	this->_errorPage[406] = "/var/www/error406.html";
// 	this->_errorPage[413] = "/var/www/error413.html";
// 	this->_errorPage[500] = "/var/www/error500.html";
// }

/**
 * @brief	Sets up max client HTTP request size to e.g., prevent DOS attacks.
 * 			Default is set to 1MB. Prefixes not supported (e.g., GB, MiB, etc).
 * SYNTAX:	client_max_body_size [size]
 */
void	Block::setClientMaxBodySize(std::vector<std::string> args)
{
	if (args.size() != 1)
	{
		this->_clientMaxBodySize = 1000000;
		if (!args.empty())
			std::cerr << RED "client_max_body_size error: invalid num of arg. Resetting to default...\n" RESET;
		return ;
	}
	unsigned long long size;
	std::stringstream iss(args.at(0));
	iss >> size;
	if (iss.fail())
	{
		this->_clientMaxBodySize = 1000000;
		std::cerr << RED "client_max_body_size: invalid arg type. Resetting to default...\n" RESET;
		return ;
	}
	if (this->_clientMaxBodySize != -1)
		std::cerr << RED "Warning: client_max_body_size already exists. Overrding with new size\n" RESET;
	this->_clientMaxBodySize = size;
}

/**
 * @brief	Sets up CGI to handle non-htm/html files.
 * SYNTAX:	cgi_script [extension 1] [extension 2] [...] [path to executable]
 */
// void	Block::setCgiScript(std::vector<std::string> args)
// {
// 	if (args.size() < 2)
// 	{
// 		if (!args.empty())
// 			std::cerr << RED "cgi_script error: invalid num of args.\n" RESET;
// 		return ;
// 	}
// 	std::string	cgiPath = args.back();
// 	for (size_t i = 0; i < args.size() - 1; i++)
// 		this->_cgiScript[args[i]] = cgiPath;
// }

/**
 * @brief	Sets up the default files that should be served (in order of
 * 			preference) when a directory is requested. Default is an
 * 			index.html file.
 * SYNTAX:	index [file 1] [file 2] ...
 */
void	Block::setIndex(std::vector<std::string> args)
{
	if (args.empty()) {
		// this->_index.push_back("index.html"); #fixme ERROR HERE? Not supposed to have default index
	}
	else
	{
		if (!this->_index.empty())
			std::cerr << RED "Warning: index already exists. Overriding with new indexes...\n" RESET;
		this->_index.clear();
		for (std::vector<std::string>::iterator it = args.begin(); it != args.end(); it++)
		{
			if (std::find(_index.begin(), _index.end(), *it) == _index.end()) // Only add if it's not duplicated
				this->_index.push_back(*it);
		}
	}
}

/**
 * @brief	Controls whether a list of files present in a requested directory
 * 			is shown when a directory is requested but no index.html file is
 * 			available.
 * SYNTAX:	autoindex [on/off]
 */
void Block::setAutoindex(std::vector<std::string> args)
{
	if (args.size() != 1)
	{
		this->_autoindex = 0;
		if (!args.empty())
			std::cerr << RED "autoindex error: invalid num of args. Autoindex set to 'off'\n" RESET;
	}
	else if (args[0] == "on")
		this->_autoindex = 1;
	else if (args[0] == "off")
		this->_autoindex = 0;
	else
	{
		this->_autoindex = 0;
		std::cerr << RED "autoindex error: invalid arg type: " + args[0] + ". Autoindex set to 'off'\n" RESET;
	}
}

/**
 * @brief	Sets the root directory from which the server serves files. Can be
 * 			overriden in a location block by re-specifying the root directory.
 * SYNTAX:	root [path to root directory]
 */
void			Block::setRoot(std::vector<std::string> args)
{
	if (args.size() != 1)
	{
		if (!args.empty())
		{
			std::cerr << RED "root error: invalid num of args. Root set to " << args.at(0) << ".\n" RESET;
			this->_root = args.at(0);
		}
		else
			this->_root = "/var/www";
	}
	else
	{
		if (this->_root != "")
			std::cerr << RED "Warning: root already exists. Overriding with new root...\n" RESET;
		this->_root = args.at(0);	
	}
}

// HELPER FUNCTION FOR SETLIMITEXCEPT()
void	Block::initDefaultLimitExcept(void)
{
	this->_limitExcept.clear();
	for (std::set<std::string>::iterator it = validMethods.begin(); it != validMethods.end(); it++)
		this->_limitExcept.push_back(*it);
}

/**
 * @brief	Sets the allowed HTTP methods. Can be overriden in a specific
 * 			location block.
 * SYNTAX:	limit_except [METHOD 1] [METHOD 2] ...
 */
void	Block::setLimitExcept(std::vector<std::string> args)
{
	if (args.empty())
		initDefaultLimitExcept();
	else if (args.size() > Block::validMethods.size())
	{
		initDefaultLimitExcept();
		std::cerr << RED "limit_except error: invalid num of args. Resetting to default methods...\n" RESET;
	}
	else
	{
		this->_limitExcept.clear();
		for (std::vector<std::string>::iterator it = args.begin(); it != args.end(); it++)
		{
			if (Block::validMethods.find(*it) == Block::validMethods.end())
			{
				initDefaultLimitExcept();
				std::cerr << RED "limit_except error: invalid method: '" + *it + "'. Resetting to default methods...\n" RESET;
				return ;
			}
			this->_limitExcept.push_back(*it);
		}
	}
}

// ================================= STATIC =================================

/**
 * @brief Initialize all the valid HTTP methods (can add more later on)
 */
std::set<std::string>	Block::initValidMethods(void)
{
	std::set<std::string> ret;
	
	ret.insert("POST");
	ret.insert("GET");
	ret.insert("DELETE");
	return (ret);
}
std::set<std::string> Block::validMethods = Block::initValidMethods();

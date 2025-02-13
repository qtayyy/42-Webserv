/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationBlock.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qtay <qtay@student.42kl.edu.my>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/12 15:21:29 by qtay              #+#    #+#             */
/*   Updated: 2025/01/22 18:29:07 by qtay             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATIONBLOCK_HPP
# define LOCATIONBLOCK_HPP

#include "Block.hpp"
#include "ServerBlock.hpp"
#include "../includes/Webserv.hpp"

class ServerBlock;
class LocationBlock : public Block
{
	public:
		LocationBlock(void) : Block() {};
		LocationBlock(ServerBlock *parent) : _parentServerBlock(parent) {}
		~LocationBlock(void) {};

		int		parseLocation(std::vector<std::string> args, int i);
		void	initDefaultLocationBlockConfig(void);

		// Setters
		void	setUri(std::vector<std::string> args = std::vector<std::string>());
		void	setAlias(std::vector<std::string> args = std::vector<std::string>());
		void	setReturn(std::vector<std::string> args = std::vector<std::string>());
		void	setCgiPass(std::vector<std::string> args = std::vector<std::string>());
		void	setUploadPath(std::vector<std::string> args = std::vector<std::string>());

		// Getters
		std::string	getUri(void) { return (_uri); };
		std::string	getAlias(void) { return (_alias); };
		std::pair<int, std::string>	getReturn(void) { return (_return); };
		std::string	getCgiPass(void) { return (_cgiPass); };
		std::string	getUploadPath(void) { return (_uploadPath); };

		void	printBlock(void);

	private:
		ServerBlock	*_parentServerBlock;
		std::string	_uri; // the uri that will "trigger" this location block
		std::string	_alias; // 3rd requirement for routes (see subject PDF)
		std::pair<int, std::string>	_return; // redirection
		std::string	_cgiPass;
		std::string	_uploadPath;

		static std::map<std::string, void (LocationBlock::*)(std::vector<std::string>)>	locationParseMap;
		static std::map<std::string, void (LocationBlock::*)(std::vector<std::string>)>	initLocationMap(void);
	
		void	inheritErrorPages(std::map<int, std::string> parentErrorPages);
		void	inheritCgiPass(std::map<std::string, std::string> parentCgiScripts);
};

#endif
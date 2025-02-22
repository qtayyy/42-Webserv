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

		int		parseLocation(stringList args, int i);
		void	initDefaultLocationBlockConfig(void);
		static LocationBlock *emptyBlock;

		// Setters
		void	setUri(stringList args = stringList());
		void	setAlias(stringList args = stringList());
		void	setReturn(stringList args = stringList());
		void	setCgiPass(stringList args = stringList());

		// Getters
		string	getUri(void) { return (_uri); };
		string	getAlias(void) { return (_alias); };
		std::pair<int, string>	getReturn(void) { return (_return); };
		string	getCgiPass(void) { return (_cgiPass); };

		void	printBlock(void);

	private:
		ServerBlock	*_parentServerBlock;
		string	_uri; // the uri that will "trigger" this location block
		string	_alias; // 3rd requirement for routes (see subject PDF)
		std::pair<int, string>	_return; // redirection
		string	_cgiPass;

		static std::map<string, void (LocationBlock::*)(stringList)> locationParseMap;
		static std::map<string, void (LocationBlock::*)(stringList)> initLocationMap(void);
	
		void	inheritErrorPages(std::map<int, string> parentErrorPages);
		void	inheritCgiPass(std::map<string, string> parentCgiScripts);
};

#endif
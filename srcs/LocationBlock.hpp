/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationBlock.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qtay <qtay@student.42kl.edu.my>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/12 15:21:29 by qtay              #+#    #+#             */
/*   Updated: 2025/01/12 19:50:03 by qtay             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATIONBLOCK_HPP
# define LOCATIONBLOCK_HPP

#include "Block.hpp"
#include "../includes/Webserv.hpp"

class LocationBlock : public Block
{
	public:
		LocationBlock(void) : Block() {};
		~LocationBlock(void) {};

		// Setters
		void	setUri(std::vector<std::string> args = std::vector<std::string>());
		void	setAlias(std::vector<std::string> args = std::vector<std::string>());
		void	setReturn(std::vector<std::string> args = std::vector<std::string>());

		// Getters
		std::string	getUri(void) { return (_uri); };
		std::string	getAlias(void) { return (_alias); };
		std::pair<int, std::string>	getReturn(void) { return (_return); };

		void	printBlock(void);

	private:
		std::string	_uri; // the uri that will "trigger" this location block
		std::string	_alias; // 3rd requirement for routes (see subject PDF)
		std::pair<int, std::string>	_return; // redirection

		static std::map<std::string, void (LocationBlock::*)(std::vector<std::string>)>	locationParseMap;
		static std::map<std::string, void (LocationBlock::*)(std::vector<std::string>)>	initLocationMap(void);

};

#endif
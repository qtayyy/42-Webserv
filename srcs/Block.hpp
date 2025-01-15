/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Block.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qtay <qtay@student.42kl.edu.my>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/11 17:32:28 by qtay              #+#    #+#             */
/*   Updated: 2025/01/15 13:40:24 by qtay             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef BLOCK_HPP
# define BLOCK_HPP

#include "../includes/Webserv.hpp"
#include "Exception.hpp"

/**
 * @brief	Abstract class for the ServerBlock and LocationBlock classes.
 * 			Contains all attributes that are common to both dervied classes to
 * 			be inherited. (Discuss if we wanna write all classes in orthodox
 * 			canonical form)
 */
class Block
{
	public:
		Block(void) : _autoindex(-1), _clientMaxBodySize(0) {}; // -1 and 0 indicate they haven't been set
		virtual ~Block(void) {};

		virtual void	printBlock(void) = 0;

		// Setters (pass in as reference?)
		void			setRoot(std::vector<std::string> args = std::vector<std::string>());
		void			setErrorPage(std::vector<std::string> args = std::vector<std::string>());
		void			setClientMaxBodySize(std::vector<std::string> args = std::vector<std::string>());
		void			setCgiScript(std::vector<std::string> args = std::vector<std::string>());
		void			setAutoindex(std::vector<std::string> args = std::vector<std::string>());
		void			setIndex(std::vector<std::string> args = std::vector<std::string>());
		void			setLimitExcept(std::vector<std::string> args = std::vector<std::string>());

		// Getters (return a reference or change to a more useful return type?)
		std::string							getRoot(void) { return (this->_root); };
		std::vector<std::string>			getIndex(void) { return (this->_index); };
		std::map<int, std::string>			getErrorPage(void) { return (this->_errorPage); };
		std::vector<std::string>			getLimitExcept(void) { return (this->_limitExcept); };
		int									getAutoindex(void) { return (this->_autoindex); };
		std::map<std::string, std::string>	getCgiScript(void) {return (this->_cgiScript); };
		unsigned long long					getClientMaxBodySize(void) {return (this->_clientMaxBodySize); };

	protected:
		static std::set<std::string>	validMethods;
		static std::set<std::string>	initValidMethods(void);

		void	initDefaultLimitExcept(void); // Helper function
		void	initDefaultErrorPage(void); // Helper function (TBD)

		std::string							_root;
		std::vector<std::string>			_index;
		std::map<int, std::string>			_errorPage; // TBD
		std::vector<std::string>			_limitExcept;
		int									_autoindex;
		std::map<std::string, std::string>	_cgiScript;
		unsigned long long					_clientMaxBodySize;

};

#endif	

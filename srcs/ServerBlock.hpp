#ifndef SERVERBLOCK_HPP
# define SERVERBLOCK_HPP

#include "Block.hpp"
#include "LocationBlock.hpp"
#include "../includes/Webserv.hpp"

/**
 * @brief	Stores all information of a server block from the given config.
 * 
 * Our config file should allow us to:
 * 1.	Choose host and port for each server
 * 			a. First server will be used if there are several with the same ip:port
 * 2.	Set up server names or not
 * 3.	Set up custom error pages or not
 * 4.	Limit client body size
 * 5.	Set up routes with the following rules (no regexp):
 * 			a. Accepted HTTP methods for a specific routes (POST/GET)
 * 			b. A HTTP redirection
 * 			c. Dir/file where a file should be searched
 * 			d. Turn on/off directory listing
 * 			e. Default file for a directory
 * 			f. Execute CGI based on a specific file extension
 * 			g. Can upload files and configure where to save them
 */
class ServerBlock : public Block
{
	public:
		ServerBlock(void) : Block() {};
		~ServerBlock(void) {};

		void	printBlock(void);

		// Setters
		void	setListen(std::vector<std::string> args = std::vector<std::string>());
		void	setServerName(std::vector<std::string> args = std::vector<std::string>());
		void	setLocation(std::vector<std::string> args = std::vector<std::string>());

		// Getters
		std::vector<std::pair<uint32_t, int>>	getListen(void) { return (this->_listen); };
		std::vector<std::string>	getServerName(void) { return (this->_serverName); };

		// parseServer

	private:
		std::vector<std::pair<uint32_t, int>>	_listen; // ip:port
		std::vector<std::string>				_serverName; // set up or not (default is localhost, can we not use DNS resolution?)
		std::vector<LocationBlock>				_location;

		static std::map<std::string, void (ServerBlock::*)(std::vector<std::string>)>	serverParseMap;
		static std::map<std::string, void (ServerBlock::*)(std::vector<std::string>)>	initServerMap(void);
		uint32_t ipToInt(const std::string &ip); // helper function
};

std::string	intToIp(uint32_t ip); // private?

#endif
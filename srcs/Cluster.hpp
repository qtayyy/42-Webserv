#ifndef CLUSTER_HPP
# define CLUSTER_HPP

#include "../includes/Webserv.hpp"
#include "Exception.hpp"
#include "ServerBlock.hpp"
#include "Client.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"

#include <iostream>
#include <fstream>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

class Cluster
{
	public:
		Cluster(void) {};
		Cluster(std::string &configPath) : _configPath(configPath), _numOfFds(0) {};
		~Cluster(void);

		void						parse(void);
		std::vector<std::string>	tokenizeConfig(std::string &configPath);
		void						parseConfig(std::vector<std::string> tokens);
		void						init(void);
        void removeFd(int i);
        void run(void);

        class	ClusterException : public Exception
		{
			public:
				ClusterException(const std::string& message)
					: Exception(message) {}
		};

	private:
		std::string											_configPath;
		struct pollfd										_pollFds[1000]; // Fix at 1k?
		int													_numOfFds;

		std::vector<ServerBlock>							_servers;
		std::map<std::string, std::vector<ServerBlock *> >	_IPPortToServer;
		std::map<int, std::vector<ServerBlock *> >			_listenerToServer;

		std::map<int, Client *>								_clients;

		void	mapIPPortToServer(void);
		int		createListenerSocket(std::string IP, std::string Port);
		void	handleNewClient(int listenerFd);
};

std::string	strTrim(std::string str, std::string delims);

#endif
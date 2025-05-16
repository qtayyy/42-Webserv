/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qtay <qtay@student.42kl.edu.my>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/12 16:34:35 by qtay              #+#    #+#             */
/*   Updated: 2025/01/21 18:03:31 by qtay             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Webserv.hpp"
#include "Cluster.hpp"
#include "Log.hpp"

#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>
#include <cstdio>
#include "LocationBlock.hpp"

void deleteLogs(const std::string& folderPath) {
    DIR* dir = opendir(folderPath.c_str());
    if (!dir)
        return;

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        std::string name = entry->d_name;

        // Skip "." and ".."
        if (name == "." || name == "..")
            continue;

        std::string filePath = folderPath + "/" + name;

        if (entry->d_type == DT_DIR) {
            // Recursively delete contents
            deleteLogs(filePath);
            // Remove the directory after it's emptied
            rmdir(filePath.c_str());
        } else {
            // Check if file ends with ".log"
            if (name.size() >= 4 && name.compare(name.size() - 4, 4, ".log") == 0) {
                remove(filePath.c_str());
            }
        }
    }
    closedir(dir);
}

int	main(int argc, char **argv)
{
	deleteLogs("logs/cgi");
	deleteLogs("logs/requests");
	deleteLogs("logs/responses");
	LogStream::log("log_trace.log", std::ios::trunc) << std::endl;

	std::string	defaultPath = "conf/good/default.conf";
	if (argc > 2)
	{
		std::cerr << RED "Usage error: invalid num o f args\n" <<
		"Usage: ./webserv [config file]\n" RESET;
		return (EXIT_FAILURE);
	}
	if (argc == 2)
		defaultPath = argv[1];
	try
	{
		Cluster	cluster(defaultPath);
		cluster.parse();
		cluster.init();
		cluster.run();
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}

}

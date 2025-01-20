/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qtay <qtay@student.42kl.edu.my>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/20 13:46:42 by qtay              #+#    #+#             */
/*   Updated: 2025/01/20 14:11:42 by qtay             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"

/**
 * @brief	Converts IP addresses from integer to string.
 */
std::string	intToIp(uint32_t ip)
{
	struct in_addr	addr;
	addr.s_addr = htonl(ip);

	const char*	ipStr = inet_ntoa(addr);
	if (ipStr == NULL)
		return ("");
    return (std::string(ipStr));
}
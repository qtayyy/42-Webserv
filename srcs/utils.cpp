/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qtay <qtay@student.42kl.edu.my>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/20 13:46:42 by qtay              #+#    #+#             */
/*   Updated: 2025/01/20 13:46:43 by qtay             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"

/**
 * @brief	Converts IP addresses from integer to string.
 */
std::string	intToIp(uint32_t ip)
{
	if (ip > 4294967295) // max ip (double check)
		return ("");

	struct in_addr	addr;
	addr.s_addr = htonl(ip);

	const char*	ipStr = inet_ntoa(addr);
	if (ipStr == NULL)
		return ("");
    return (std::string(ipStr));
}
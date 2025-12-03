/******************************************************************************/
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: scarlucc <scarlucc@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/15 13:41:21 by scarlucc          #+#    #+#             */
/*   Updated: 2025/12/01 20:03:51 by scarlucc         ###   ########.fr       */
/*                                                                            */
/******************************************************************************/


#include "../includes/Server.hpp"
#include <iostream>
#include <cstdlib>
#include <csignal>
#include <bits/stdc++.h>

void	handle_signal(int sig)
{
	(void)sig;
	throw std::runtime_error("\nShutting down server...");
}

int main(int argc, char **argv)
{
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <port> <password>\n";
        return 1;
    }

	signal(SIGINT,	handle_signal);
	
    std::string port_str = argv[1];
	for (size_t i = 0; i < port_str.length(); i++)
	{
		if (!isdigit(port_str[i]))
		{
			std::cerr << "Usage: " << argv[0] << " <port> <password>\n";
			std::cerr << "Port number must be between 6000 and 6669\n";
        	return 1;
		}
	}
	
	char *end_p;
	long value = std::strtol(argv[1], &end_p, 10);
	if (value < 6000 || value > 6669)
	{
		std::cerr << "Usage: " << argv[0] << " <port> <password>\n";
		std::cerr << "Port number must be between 6000 and 6669\n";
		return 1;
	}
	int port = static_cast<int>(value);
	
    std::string pw = argv[2];
	if (pw.length() <= 0 || pw.length() >= 8 || pw.find(" "))
	{
		std::cerr << "Usage: " << argv[0] << " <port> <password>\n";
		std::cerr << "password must be between 1 and 8 characters and no spaces\n";
		return 1;
	}

    try {
        Server serv(port, pw);
        serv.run();
    } catch (const std::exception &e) {
        std::cerr << "\nFatal: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}

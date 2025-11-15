/******************************************************************************/
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: scarlucc <scarlucc@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/15 13:41:21 by scarlucc          #+#    #+#             */
/*   Updated: 2025/11/15 13:41:31 by scarlucc         ###   ########.fr       */
/*                                                                            */
/******************************************************************************/


#include "../includes/Server.hpp"
#include <iostream>
#include <cstdlib>

int main(int argc, char **argv)
{
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <port> <password>\n";
        return 1;
    }
    int port = std::atoi(argv[1]);
    std::string pw = argv[2];

    try {
        Server serv(port, pw);
        serv.run();
    } catch (const std::exception &e) {
        std::cerr << "Fatal: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}

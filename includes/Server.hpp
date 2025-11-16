/******************************************************************************/
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: scarlucc <scarlucc@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/15 11:46:27 by scarlucc          #+#    #+#             */
/*   Updated: 2025/11/16 11:39:44 by scarlucc         ###   ########.fr       */
/*                                                                            */
/******************************************************************************/

#ifndef SERVER_HPP
#define SERVER_HPP
#define _ITERATOR_DEBUG_LEVEL 1
#define _HAS_ITERATOR_DEBUGGING 1

#include <string>
#include <vector>
#include <map>
#include <poll.h>
#include <stdio.h>
#include <errno.h>
#include <sstream>

class Client;

class Server
{
private:
    int _port;
    std::string _password;
    int _server_fd;

    std::vector<struct pollfd> _pfds;
    std::map<int, Client*> _clients; // key: fd

	std::map<std::string, bool (Server::*)(int, std::vector<std::string>)> _commands;

    int set_nonblocking(int fd);
    int make_server_socket(int port);

	void Server::command_map();
	bool Server::handle_command(int fd, const std::vector<std::string> &line);

    // helpers
    std::vector<std::string> split(const std::string &s, const std::string &delim);

public:
    Server(int port, const std::string &password);
    ~Server();

    // main loop
    void run();

    // internal actions
    void accept_new_connection();
    void handle_client_read(int idx); // idx = index in _pfds
    void close_client(int idx); // index in _pfds

    // broadcast
    void broadcast_from(int sender_fd, const std::string &msg);

	//commands
	bool	pass(int fd, std::vector<std::string> vect);
	bool	nick(int fd, std::vector<std::string> vect);
	bool	user(int fd, std::vector<std::string> vect);
	bool	quit(int fd, std::vector<std::string> vect);
};

#endif

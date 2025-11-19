/******************************************************************************/
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: scarlucc <scarlucc@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/15 11:46:27 by scarlucc          #+#    #+#             */
/*   Updated: 2025/11/19 12:52:47 by scarlucc         ###   ########.fr       */
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

	void command_map();
	bool handle_command(int fd, const std::vector<std::string> &line);

    // helpers
    std::vector<std::string> split(const std::string &s, const std::string &delim);
    std::vector<std::string> split2(std::string str, char c, size_t pos);
    size_t strlen(char *s){size_t i = 0; while (s[i++]){}; return (i);}
    Client  *find_by_nick(std::string &name);

    
public:
    Server(int port, const std::string &password);
    ~Server();

    // main loop
    void run();

    // internal actions
    void accept_new_connection();
    void handle_client_read(int fd);
    void close_client(int idx); // index in _pfds

    // broadcast
    void broadcast_from(int sender_fd, const std::string &msg);

	//commands
    bool    privMsg(int fd, std::vector<std::string> vect);
	bool	pass(int fd, std::vector<std::string> vect);
	bool	nick(int fd, std::vector<std::string> vect);
	bool	user(int fd, std::vector<std::string> vect);
	bool	quit(int fd, std::vector<std::string> vect);
};

#endif

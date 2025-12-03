/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: negambar <negambar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/15 11:46:27 by scarlucc          #+#    #+#             */
/*   Updated: 2025/12/03 12:02:07 by negambar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP
#define _ITERATOR_DEBUG_LEVEL 1
#define _HAS_ITERATOR_DEBUGGING 1

#include <string>
#include <vector>
#include <map>
#include <poll.h>
#include <iostream>
#include <stdio.h>
#include <errno.h>
#include <sstream>
#include <algorithm>

class Client;
class Channel;

class Server
{
private:
    int _port;
    std::string _password;
    int _server_fd;

    std::vector<struct pollfd> _pfds;
    std::map<int, Client*> _clients; // key: fd
    std::map<std::string, Channel *> _channels; //key: nome canale

	std::map<std::string, bool (Server::*)(int, std::vector<std::string>)> _commands;

    int set_nonblocking(int fd);
    int make_server_socket(int port);

	void command_map();
	bool handle_command(int fd, const std::vector<std::string> &line);

    
    
public:
    Server(int port, const std::string &password);
    ~Server();
    
    Client                      *find_by_nick(std::string &name);
    Channel                     *find_channel_name(std::string &name);
    Channel                     *find_user_channel(Client &c, std::string &name); //finds channel if user is in it
    void                        add_channel(std::string name);
    std::string                 getNamesMessage(Channel *channel, int client_fd);
    
    // main loop
    void run();
    
    // internal actions
    void       handle_client_read(int fd);
    void    accept_new_connection();
    void    close_client(int idx); // index in _pfds
    bool    send_to_channel(int fd, std::string recipient, std::vector<std::string> parts);
    bool    send_to_channel(int fd, std::string recipient, std::string msg);
	//forse sarebbe meglio mettere funzioni send_to_channel nel canale
    void    remove_channel(std::string name);
    // broadcast
    void broadcast_from(int sender_fd, const std::string &msg);
    
	//commands
    bool    privMsg(int fd, std::vector<std::string> vect);
	bool	pass(int fd, std::vector<std::string> vect);
	bool	nick(int fd, std::vector<std::string> vect);
	bool	user(int fd, std::vector<std::string> vect);
	bool	quit(int fd, std::vector<std::string> vect);
    bool    join(int fd, std::vector<std::string> vect);
    bool    names(int fd, std::vector<std::string> name);
    bool    mode(int fd, std::vector<std::string> parts);
    bool    invite(int fd, std::vector<std::string> parts);
    bool    part(int fd, std::vector<std::string> parts);
    bool    kick(int fd, std::vector<std::string> parts);
    bool    topic(int fd, std::vector<std::string> params);
    // helpers
    void                        write_to_client(int fd, std::string msg);
    std::vector<std::string>    split(const std::string &s, const std::string &delim);
    std::vector<std::string>    split2(std::string str, char c, size_t pos);
    size_t                      strlen(char *s){size_t i = 0; while (s[i++]){}; return (i);}
    void                        kick_in_loop(int client, std::string kick_chan, std::string kick_clients, std::string msg);
    bool                        validate_mode_syntax(Client &c, std::vector<std::string> &parts);

    std::string                 unresplit(std::vector<std::string> v)
    {
        std::string result;
        for (size_t i = 0; i < v.size(); ++i)
        {
            result += v[i];
            if (i + 1 < v.size())
                result += " ";
        }
        return (result);
    };
};

#endif

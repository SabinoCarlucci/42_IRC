/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: negambar <negambar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/15 13:38:41 by scarlucc          #+#    #+#             */
/*   Updated: 2025/11/16 13:20:33 by negambar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "../includes/Server.hpp"
#include "../includes/Client.hpp"

#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <cstring>
#include <sys/socket.h>

Server::Server(int port, const std::string &password)
: _port(port), _password(password), _server_fd(-1)
{
    _server_fd = make_server_socket(_port);
    if (_server_fd < 0) {
        std::cerr << "Failed to create server socket\n";
        throw std::runtime_error("socket");
    }

    struct pollfd p;
    p.fd = _server_fd;
    p.events = POLLIN;
    p.revents = 0;
    _pfds.push_back(p);

	command_map();

    std::cout << "Server listening on port " << _port << "\n";
}

Server::~Server()
{
    for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        close(it->first);
        delete it->second;
    }
    _clients.clear();
    for (size_t i = 0; i < _pfds.size(); ++i) close(_pfds[i].fd);
}

int Server::set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) return -1;
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) return -1;
    return 0;
}

int Server::make_server_socket(int port) {
    int srv = socket(AF_INET, SOCK_STREAM, 0);
    if (srv < 0) { perror("socket"); return -1; }

    int opt = 1;
    setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(srv, (struct sockaddr*)&addr, sizeof(addr)) < 0) { perror("bind"); close(srv); return -1; }
    if (listen(srv, 10) < 0) { perror("listen"); close(srv); return -1; }
    if (set_nonblocking(srv) < 0) { perror("set_nonblocking"); close(srv); return -1; }

    return srv;
}

void Server::run()
{
    while (true) {
        int rv = poll(_pfds.empty() ? NULL : &_pfds[0], _pfds.size(), -1);
        if (rv < 0) {
            if (errno == EINTR) continue;
            perror("poll");
            break;
        }
        // handle events
        // first entry: server socket
        if (!_pfds.empty() && (_pfds[0].revents & POLLIN)) {
            accept_new_connection();
            _pfds[0].revents = 0;
        }
        // then clients
        for (size_t i = 1; i < _pfds.size(); ++i) {
            if (_pfds[i].revents == 0) continue;
            if (_pfds[i].revents & (POLLERR | POLLHUP | POLLNVAL)) {
                close_client(i);
                --i;
                continue;
            }
            if (_pfds[i].revents & POLLIN) {
                handle_client_read(i);
				continue; //dovrebbe evitare segfault in seguito a QUIT
			}
			if (_pfds.size() > i)
            	_pfds[i].revents = 0;
        }
    }
}

void Server::accept_new_connection()
{
    struct sockaddr_in cli_addr;
    socklen_t cli_len = sizeof(cli_addr);
    int client_fd = accept(_server_fd, (struct sockaddr*)&cli_addr, &cli_len);
    if (client_fd < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) return;
        perror("accept");
        return;
    }
    if (set_nonblocking(client_fd) < 0) {
        perror("set_nonblocking(client)");
        close(client_fd);
        return;
    }
    
    char ipbuf[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(cli_addr.sin_addr), ipbuf, INET_ADDRSTRLEN);

    Client *c = new Client(client_fd);
    c->set_hostname(ipbuf);
    _clients[client_fd] = c;

    struct pollfd p;
    p.fd = client_fd;
    p.events = POLLIN;
    p.revents = 0;
    _pfds.push_back(p);

    inet_ntop(AF_INET, &(cli_addr.sin_addr), ipbuf, INET_ADDRSTRLEN);
    std::cout << "Accepted client fd=" << client_fd << " ip=" << ipbuf
              << " port=" << ntohs(cli_addr.sin_port) << std::endl;
}

void Server::handle_client_read(int idx)
{
    int fd = _pfds[idx].fd;
    char buf[4096];
    ssize_t n = recv(fd, buf, sizeof(buf), 0);
    if (n > 0) {
        _clients[fd]->buffer().append(buf, n);
        // process complete lines (support \n or \r\n)
        std::string &b = _clients[fd]->buffer();
        size_t pos;
        while ((pos = b.find('\n')) != std::string::npos) {
            std::string line = b.substr(0, pos + 1);
            // remove trailing \r and \n
			std::size_t last;//trova posizione di ultimo \n o \r
            while (!line.empty() && ((line.size() - 1) == (last = line.find_last_of("\n\r"))))
				line.erase(line.size() - 1);//se ultimo \r o \n e' a fine stringa, rimuovi ultimo carattere stringa

            if (!line.empty()) {
				std::cout << "line = " << line <<std::endl;
				//const char *s = line.c_str();//per vedere in debug
                std::cout << "[RECV fd=" << fd << "] " << line << std::endl;
                // minimal parsing: split at space
                std::vector<std::string> parts = split(line, " ");
                if (!parts.empty()) {
                    bool ret = handle_command(fd, parts);
                    if (!ret)
                        send(fd, "ciao\n", 6, 0);
                    else
                        return;
                    //std::string cmd = parts[0];
                    /* if (cmd == "PASS") {
					    std::string cl_psw = parts[1];
						if (cl_psw)
                        if (parts.size() >= 2 && parts[1] == _password) {
                            _clients[fd]->set_authenticated();
                            std::string reply = "Password accepted";
                            send(fd, reply.c_str(), reply.size(), 0);
                        } else {
                            std::string reply = "Password wrong";
                            send(fd, reply.c_str(), reply.size(), 0);
                        }
                    } else if (cmd == "NICK") {
                        if (parts.size() >= 2) {
                            std::map<int, Client *>::iterator it = _clients.begin();
                            for (; it != _clients.end(); ++it)
                            {
                                if (it->first != fd && it->second->get_nick() == parts[1])
                                {
                                    send(fd, "Nickname already taken\n", 23, 0);
                                    break;
                                }
                            }
                            if (it == _clients.end())
                            {
                            _clients[fd]->set_nick(parts[1]);
                            std::string reply = "Nickname set to " + parts[1] + "\n";
                            send(fd, reply.c_str(), reply.size(), 0);
                            }
                        }
                    } else if (cmd == "USER") {
                        if (parts.size() >= 2) {
                            _clients[fd]->set_user(parts[1]);
                            std::string reply = "User set to " + parts[1] + "\n";
                            send(fd, reply.c_str(), reply.size(), 0);
                        }
                    } else if (cmd == "QUIT") {
                        std::string goodbye = "Goodbye\n";
                        send(fd, goodbye.c_str(), goodbye.size(), 0);
                        close_client(idx);
                        return;
                    } else {
                        // broadcast simple message to others
						std::string fd_to_string;
						std::ostringstream convert;
						convert << fd;
						fd_to_string = convert.str();
                        std::string msg = "[" + (_clients[fd]->get_nick().empty() ? fd_to_string : _clients[fd]->get_nick()) + "]: " + line + "\n";
                        broadcast_from(fd, msg);
                    } */
                }
            }
            b.erase(0, pos + 1);
        }
    }else if (n == 0) {
        std::cout << "Client fd=" << fd << " disconnected\n";
        close_client(idx);
    } else {
        perror("recv");
        close_client(idx);
    }
}

void Server::close_client(int idx)
{
    if (idx >= (int)_pfds.size()) return;
    int fd = _pfds[idx].fd;
    // remove from pfds
    _pfds.erase(_pfds.begin() + idx);
    // delete client object and close fd
    std::map<int, Client*>::iterator it = _clients.find(fd);
    if (it != _clients.end()) {
        delete it->second;
        _clients.erase(it);
    }
    close(fd);
    std::cout << "Closed client fd=" << fd << std::endl;
}

void Server::broadcast_from(int sender_fd, const std::string &msg)
{
    for (size_t i = 1; i < _pfds.size(); ++i) {
        int fd = _pfds[i].fd;
        if (fd == sender_fd) continue;
        send(fd, msg.c_str(), msg.size(), 0);
    }
}

std::vector<std::string> Server::split(const std::string &s, const std::string &delim)
{
    std::vector<std::string> elems;
    size_t start = 0;
    size_t pos;
    while ((pos = s.find(delim, start)) != std::string::npos) {
        if (pos > start) elems.push_back(s.substr(start, pos - start));
        start = pos + delim.length();
    }
    if (start < s.size()) elems.push_back(s.substr(start));
    return elems;
}

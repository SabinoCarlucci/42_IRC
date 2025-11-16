/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: negambar <negambar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/15 12:05:11 by scarlucc          #+#    #+#             */
/*   Updated: 2025/11/16 11:46:41 by negambar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Client.hpp"

Client::Client(int fd)
: _fd(fd),
  _buffer(),
  _nick(),
  _user(),
  _authenticated(false)
{}

Client::~Client()
{}

int Client::get_client_fd() const { return _fd; }

bool Client::get_authenticated() const
{
    return _authenticated;
}
void Client::set_authenticated() { _authenticated = true; }

std::string Client::get_nick() const { return _nick; }
void Client::set_nick(const std::string &n) { _nick = n; }

std::string Client::get_user() const { return _user; }
void Client::set_user(const std::string &u) { _user = u; }

std::string &Client::buffer() { return _buffer; }

void Client::set_hostname(const std::string &h) {_hostname = h;}
std::string &Client::get_hostname() {return (_hostname);}
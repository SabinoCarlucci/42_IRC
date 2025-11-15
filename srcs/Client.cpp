/******************************************************************************/
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: scarlucc <scarlucc@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/15 12:05:11 by scarlucc          #+#    #+#             */
/*   Updated: 2025/11/15 12:42:01 by scarlucc         ###   ########.fr       */
/*                                                                            */
/******************************************************************************/

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

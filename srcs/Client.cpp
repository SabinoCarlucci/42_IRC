/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: negambar <negambar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/15 12:05:11 by scarlucc          #+#    #+#             */
/*   Updated: 2025/11/17 18:04:27 by negambar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Client.hpp"
#include "../includes/Channel.hpp"

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


bool Client::send_message(std::string message, int fd)
{
	message += "\r\n";
	if (send(fd, message.c_str(), message.size(), 0) == -1)
	{
		return false;
	}
	return true;
}

Channel *Client::isInChannel(std::string name)
{
	std::vector<Channel *>::iterator it = _channels.begin();
	for (; it != _channels.end(); ++it)
	{
		if ((*it)->get_name() == name)
		{
			return (*it);
		}
	}
	return NULL;
}
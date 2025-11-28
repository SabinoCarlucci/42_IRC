/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cmd_helpers.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: negambar <negambar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/18 11:32:37 by negambar          #+#    #+#             */
/*   Updated: 2025/11/28 15:58:27 by negambar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Server.hpp"
#include "../includes/Channel.hpp"
#include "../includes/Client.hpp"

bool Server::send_to_channel(int fd, std::string recipient, std::vector<std::string> parts)
{
	Channel	*receiver = find_channel_name(recipient);
	Client *sender = _clients[fd]; // Get sender once

	if (!receiver)
	{
		std::string err = "401 " + recipient + " :No such channel\r\n";
		send(fd, err.c_str(), err.size(), 0);
		return false;
	}

	if (sender->isInChannel(receiver->get_name()) == NULL) 
	{
		// 442 ERR_NOTONCHANNEL
		sender->send_message(":irc 442 " + sender->get_nick() + " " + recipient + " :You're not on that channel\r\n", fd);
		return false;
	}

    std::string prefix = ":" + sender->get_nick() + "!" + sender->get_user() + "@" + sender->get_hostname();

    std::string full_message = prefix + " PRIVMSG " + recipient + " " + parts[2] + "\r\n";

    std::vector<std::string> client_nicks = receiver->get_clients();

	for (std::vector<std::string>::iterator nick_it = client_nicks.begin(); nick_it != client_nicks.end(); ++nick_it)
	{
	    std::string nick_name = *nick_it;
	    Client *client = find_by_nick(nick_name); 
	
	    if (client != _clients[fd])
	    {
	        if (!client->send_message(full_message, client->get_client_fd()))
	            return false;
	    }
	}
    
    return true;
}

bool Server::send_to_channel(int fd, std::string recipient, std::string msg)
{
    Channel *receiver = find_channel_name(recipient);
    Client  *sender   = _clients[fd];

	if (!sender)
		return (false);	
    if (!receiver)
    {
        std::string err = ":irc 401 " + sender->get_nick() +
                          " " + recipient + " :No such channel\r\n";
        send(fd, err.c_str(), err.size(), 0);
        return false;
    }

    if (sender->isInChannel(receiver->get_name()) == NULL)
    {
        std::string err = ":irc 442 " + sender->get_nick() +
                          " " + recipient + " :You're not on that channel\r\n";
        sender->send_message(err, fd);
        return false;
    }
    std::string prefix = ":" + sender->get_nick() + "!" +
                         sender->get_user() + "@" +
                         sender->get_hostname();

    std::string outgoing;
	if (msg.empty() || msg[0] == ':')
		outgoing = msg;
	else
		outgoing = prefix + " PRIVMSG " + recipient + " " + msg + "\r\n";
    std::vector<std::string> clients = receiver->get_clients();

    for (size_t i = 0; i < clients.size(); ++i)
    {
        Client *c = find_by_nick(clients[i]);
        if (!c)
			continue;
		if (c->get_client_fd() == fd)
			continue;
        if (!c->send_message(outgoing, c->get_client_fd()))
            return false;
    }
    return true;
}


void Server::add_channel(std::string name)
{
    if (_channels.find(name) == _channels.end())
        _channels[name] = new Channel(name, this);
}

std::string Server::getNamesMessage(Channel *channel, int client_fd)
{
    // The channel is guaranteed to exist here, so we don't need to check for null.
    std::vector<std::string> clients = channel->get_clients();
    std::string nick = _clients[client_fd]->get_nick();
    std::string channel_name = channel->get_name();

    // 1. Start the 353 message (RPL_NAMREPLY)
    // Format: :servername 353 <nick> = <channel> :[ @nick1 nick2 ]
    std::string message = ":irc 353 " + nick + " = " + channel_name + " :";

    // 2. Append the list of nicks
    // Note: You may want to add prefixes (@) for operators users.
    for (size_t i = 0; i < clients.size(); ++i) {
		std::string prefix = "";
		if (channel->is_op(clients[i]))
			prefix = "@";
			
        message += prefix + clients[i];
        if (i < clients.size() - 1) {
            message += " ";
        }
    }

    // message += "\r\n";
    return message;
}

bool Server::names(int fd, std::vector<std::string> name)
{
	if (_channels.find(name[1]) == _channels.end())
	{
		std::string err = "403 " + _clients[fd]->get_nick() + " " + name[1] + " :No such channel\r\n";
		_clients[fd]->send_message(err, fd);
		return(false);
	}

	Channel *channel = _channels[name[1]];
	
	 std::string names_reply = getNamesMessage(channel, fd);
	if (!_clients[fd]->send_message(names_reply, fd))
		return (false);
	
	std::string end = ":irc 366 " + _clients[fd]->get_nick() + " " + name[1] + " :End of /NAMES list";
	if (!_clients[fd]->send_message(end, fd))
		return (false);
	return (true);
}

bool	Channel::is_invited(std::string name)
{
	std::vector<std::string>::iterator it = _invites.begin();
	for (; it != _invites.end(); ++it)
	{
		if (*it == name)
			return true;
	}
	return (false);
}


void	Channel::join_channel(Client &c, std::vector<std::string> parts, int fd)
{
	if (_modes['l'] && _clients.size() >= static_cast<size_t>(_modes['l']))
	{
		c.send_message(":irc 471 " + c.get_nick() + " " + _name + " :Cannot join channel (+l)", c.get_client_fd());
		return;
	}
	if (_modes['i'] && std::find(_invites.begin(), _invites.end(), c.get_nick()) == _invites.end())
	{
		if (is_invited(c.get_nick()))
			remove_invite(c.get_nick());
		else
		{
			c.send_message(":irc 473 " + c.get_nick() + " " + _name + " :Cannot join channel (+i)\r\n", c.get_client_fd());
			return;
		}
	}
	if (_modes['k'])
	{
		if (parts.size() < 3){
			c.send_message(":irc 461 " + c.get_nick() + " JOIN :not enough parameters", c.get_client_fd());
			return;
		}
		if (parts[2] != _pass)
		{
			c.send_message(":irc 475" + c.get_nick() + " " + _name + " :Cannot join channel (+k)", c.get_client_fd());
			return;
		}
	}
	this->add_clients(c.get_nick());
	c.add_client_pointer(this);

	std::string join_msg = ":" + c.get_nick() + "!" + c.get_user() + 
                           "@" + c.get_hostname() + " JOIN :" + _name + "\r\n";

    c.send_message(join_msg, c.get_client_fd());
	serv->send_to_channel(c.get_client_fd(), get_name(), join_msg);
	send_modes(c, c.get_client_fd());
	serv->names(fd, parts);
	std::string full = ":" + c.get_nick() + "!" + c.get_user() + "@" + c.get_hostname() + " JOIN :" + _name;  // \r\n vengono aggiunti in send_to_all
	send_to_all( full );
	//c.send_message(full, c.get_client_fd());

	// Send NAMES (353) and end of NAMES (366) to the joining client
	std::vector<std::string> clients = this->get_clients();
	std::string names_list = ":irc 353 " + c.get_nick() + " = " + _name + " :";
	for (size_t i = 0; i < clients.size(); ++i)
	{
		names_list += clients[i];
		if (i < clients.size() - 1)
			names_list += " ";
	}
	//names_list += "\r\n"; //commentato per togliere rigo vuoto tra irc 353 e irc 366
	c.send_message(names_list, c.get_client_fd());

	std::string end = ":irc 366 " + c.get_nick() + " " + _name + " :End of /NAMES list\r\n";
	c.send_message(end, c.get_client_fd());
	//serv->names(fd, parts);
}

void	Channel::topuc(Client &client, std::string parameters)
{
	bool is_mod = _ops.find(client.get_nick()) != _ops.end();
	if (parameters.find(":") != std::string::npos)
		parameters = parameters.substr(parameters.find(":") + 1, parameters.size());
	if (_modes['t'] == 0)
		_topic = parameters;
	else if (is_mod)
		_topic = parameters;
	else
	{
		client.send_message(":irc 482 " + client.get_nick() + " " + _name + " :You're not a channel operator", client.get_client_fd());
		return;
	}
	serv->send_to_channel(client.get_client_fd(), this->get_name() ,":" + client.get_nick() + "!" + client.get_user() + "@" + client.get_hostname() + " TOPIC " + _name + " :" + _topic + "\r\n");
}
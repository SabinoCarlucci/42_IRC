/******************************************************************************/
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cmd_helpers.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: scarlucc <scarlucc@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/18 11:32:37 by negambar          #+#    #+#             */
/*   Updated: 2025/11/28 14:15:39 by scarlucc         ###   ########.fr       */
/*                                                                            */
/******************************************************************************/

#include "../includes/Server.hpp"
#include "../includes/Channel.hpp"
#include "../includes/Client.hpp"


/* bool	Server::send_to_channel(int fd, std::string recipient, std::vector<std::string> parts)
{
	Channel	*receiver = find_channel_name(recipient);
	if (!receiver)
	{
		std::string err = "401 " + recipient + " :No such channel\r\n";
		send(fd, err.c_str(), err.size(), 0);
		return false;
	}
    std::vector<std::string> clients = receiver->get_clients();
	if (!_clients[fd]->isInChannel(receiver->get_name())) //controlla se client e' nel canale
	{
		_clients[fd]->send_message(":irc 442 " + _clients[fd]->get_nick() + " :You're not on that channel", fd);
		return false;
	}
	for (std::map<int, Client *>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		std::string name = it->second->get_nick();
		if (name != _clients[fd]->get_nick())
			if (!this->find_by_nick(name)->send_message(parts[2], fd))
				return (false);
	}
	return (true);
} */

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

bool Server::send_to_channel(int fd, std::string recipient, std::string msg, bool raw)
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

    if (raw)
		outgoing = msg;
    else
        outgoing = prefix + " PRIVMSG " + recipient + " :" + msg + "\r\n";

    std::vector<std::string> clients = receiver->get_clients();

    for (size_t i = 0; i < clients.size(); ++i)
    {
        Client *c = find_by_nick(clients[i]);
        if (!c) continue;

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
        // If you implement channel modes, you would check them here.
        message += clients[i];
        if (i < clients.size() - 1) {
            message += " ";
        }
    }

    message += "\r\n";
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
	
	std::string end = ":irc 366 " + _clients[fd]->get_nick() + " " + name[1] + " :End of /NAMES list\r\n";
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


void	Channel::join_channel(Client &c/* , std::vector<std::string> parts, int fd */) //cambiato prototipo join_channel per evitare doppio messaggio di entrata
{
	// Use the provided Client reference 'c' as the joining client.
	// Check invite-only mode
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
	// Add the client to this channel and register the channel in the client
	this->add_clients(c.get_nick());
	c.add_client_pointer(this);

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
	serv->send_to_channel(client.get_client_fd(), this->get_name() ,":" + client.get_nick() + "!" + client.get_user() + "@" + client.get_hostname() + " TOPIC " + _name + " :" + _topic + "\r\n", true);
}
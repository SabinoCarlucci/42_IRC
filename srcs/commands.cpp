/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   commands.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: negambar <negambar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/16 11:00:25 by scarlucc          #+#    #+#             */
/*   Updated: 2025/11/28 13:01:58 by negambar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Server.hpp"
#include "../includes/Channel.hpp"
#include "../includes/Client.hpp"

void Server::command_map()
{
	_commands["NICK"] = &Server::nick;
	_commands["PASS"] = &Server::pass;
	_commands["USER"] = &Server::user;
	_commands["QUIT"] = &Server::quit;
	_commands["PRIVMSG"] = &Server::privMsg;
	_commands["privmsg"] = &Server::privMsg;
	_commands["JOIN"] = &Server::join;
	_commands["NAMES"] = &Server::names;
	_commands["MODE"] = &Server::mode;
	_commands["INVITE"] = &Server::invite;
	_commands["PART"] = &Server::part;
}

bool Server::handle_command(int fd, const std::vector<std::string> &line)
{
	if (_commands.find(line[0]) != _commands.end())
	{
        //std::cout << "ENTRA" << std::endl;
		return (this->*(_commands[line[0]]))(fd, line);
	}
	else
	{
		std::string tmp = "temporary error: " + line[0] + "comando sconosciuto";
		//setOutbuf(fd, tmp);
		return (false);//stesso return (false) per comando non trovato e per comando trovato ma andato male. Cambiare?
	}
	return (false); //forse cambiare	
}

bool	Server::nick(int fd, std::vector<std::string> vect)
{
	if (vect.size() >= 2) 
	{
		std::map<int, Client *>::iterator it = _clients.begin();
		for (; it != _clients.end(); ++it)
		{
			if (it->first != fd && it->second->get_nick() == vect[1])
			{
				send(fd, "Nickname already taken\n", 24, 0);
				return (false);
			}
		}
		if (it == _clients.end())
		{
		_clients[fd]->set_nick(vect[1]);
		std::string reply = "Nickname set to " + vect[1] + "\n";
		send(fd, reply.c_str(), reply.size(), 0);
		}
		return (true);
	}
	std::string errmsg = "NICK: not enough parameters\n";//messaggio di errore da cambiare
	send(fd, errmsg.c_str(), errmsg.size(), 0);
	return (false);
}

bool	Server::pass(int fd, std::vector<std::string> vect)
{
	if (vect.size() >= 2 && vect[1] == _password) 
	{
		_clients[fd]->set_authenticated();
		std::string reply = "Password accepted\r\n";
		send(fd, reply.c_str(), reply.size(), 0);
		return (true);
	} 
	else 
	{
		std::string reply = "Password wrong\n";//serve messaggio di errore in caso non diamo pasword?
		send(fd, reply.c_str(), reply.size(), 0);
		return (false);
	}
}

bool	Server::user(int fd, std::vector<std::string> vect)
{
	if (vect.size() >= 2)
	{
		_clients[fd]->set_user(vect[1]);
		std::string reply = "User set to " + vect[1] + "\n";
		send(fd, reply.c_str(), reply.size(), 0);
		return (true);
	}
	std::string errmsg = "USER: not enough parameters\n";//messaggio di errore da cambiare
	send(fd, errmsg.c_str(), errmsg.size(), 0);
	return (false);
}

bool	Server::quit(int fd, std::vector<std::string> vect)
{
	//Client *client = _clients[fd];//per accedere alle funzioni di client
	std::string goodbye = "Goodbye";
	if (vect.size() > 1 && vect[1][0] == ':')
		goodbye = vect[1];
	goodbye.append("\n");
	
	//cambia con ciclo per inviare a tutti utenti in tutti canali di tizio quit
	send(fd, goodbye.c_str(), goodbye.size(), 0);
	close_client(fd);
	return (true);
}

bool Server::privMsg(int fd, std::vector<std::string> parts)
{
    // 1. Check for parameters (needs target and message)
    if (parts.size() < 3 || parts[2][0] != ':')
    {
        send(fd, "412 :No text to send\r\n", 22, 0); // Example: ERR_NOTEXTTOSEND (412)
        return false;
    }

    // parts[1] is the recipient's nickname/channel
    std::string recipient = parts[1];
    
	if (recipient[0] == '#' || recipient[0] == '#')
		return (send_to_channel(fd, recipient, parts));

    // 2. Find the recipient (this is your existing find logic, or use a better lookup)
    Client *receiver = find_by_nick(recipient);
    if (!receiver)
    {
		std::string err = "401 " + recipient + " :No such nick\r\n";
		send(fd, err.c_str(), err.size(), 0);
		return false;
    }
    // 3. Construct the full message
    Client *sender = _clients[fd];
    std::string prefix = ":" + sender->get_nick() + "!" + sender->get_user() + "@" + sender->get_hostname();
    // Send the message as originally received to the recipient
    std::string full_message = prefix + " PRIVMSG " + recipient + " " + parts[2] + "\r\n";
    // 4. Send the message
    if (!receiver->send_message(full_message, receiver->get_client_fd())) // Pass the receiver's FD
    	return false;
	    
    return true;
}

bool	Server::join(int fd, std::vector<std::string> parts)
{
	if (parts.size() < 2 || parts[1][0] != '#')
	{
		send(fd, " :No channel to join\r\n", 26, 0);
		return (false);
	}
	std::string key;
	if (parts.size() > 2)
		key = parts[2];
	else
		key = "";
	std::string joinChannel = parts[1];
	Client *sender = _clients[fd];

	Channel *receiver = find_channel_name(joinChannel);
	if (!receiver)
    {
        add_channel(joinChannel);
        Channel *new_channel = _channels[joinChannel];
        new_channel->add_clients(_clients[fd]->get_nick());
        _clients[fd]->add_client_pointer(new_channel);
        std::string join_msg = ":" + sender->get_nick() + "!" + sender->get_user() + 
                               "@" + sender->get_hostname() + " JOIN :" + joinChannel + "\r\n";
        sender->send_message(join_msg, fd); 
		_channels[joinChannel]->add_op(sender->get_nick());
		_clients[fd]->send_message(":" + sender->get_nick() + " MODE " + joinChannel + " +o " + sender->get_nick() + "\r\n", fd);
        names(fd, parts); 
	}
	else
	{
		Client *sender = _clients[fd];
    	if (sender->isInChannel(joinChannel) != NULL)
    	    return (true);
		_channels[joinChannel]->join_channel(*sender, parts, fd);
	}
	return (true);
}

bool Server::mode(int fd, std::vector<std::string> parts)
{
	Client	*client = _clients[fd];
	if (parts.size() < 2)
	{
		client->send_message(":irc 461 " + client->get_nick() + " MODE :Not enough parameters", fd);
		// delete	client;
		return (false);
	}
	if (parts.size() == 2)
	{
		Channel *chan = find_channel_name(parts[1]);
		if (chan)
			chan->send_modes(*client, fd);
		return (true);
	}
	if (parts.size() < 3) 
		parts.push_back("");
	if (find_by_nick(parts[1]))
		return (true);
	bool ret = _channels[parts[1]]->modify_mode(parts, *_clients[fd], fd);
	return ret;
}

bool	Server::invite(int fd, std::vector<std::string> parts)
{
	if (parts.size() < 3)
	{
		_clients[fd]->send_message(":irc 461 " + _clients[fd]->get_nick() + " INVITE :Not enough parameters", fd);
		return (false);
	}
	Client *sender = _clients[fd];
	Client *receiver = find_by_nick(parts[1]);
	Channel *chan = find_channel_name(parts[2]);
	if (!receiver)
	{
		sender->send_message(":irc 401 " + sender->get_nick() + " " + parts[1] + " :No such nick/channel\r\n", fd);
        return false;
	}
	if (!chan)
    {
        sender->send_message(":irc 403 " + sender->get_nick() + " " + parts[2] + " :No such channel\r\n", fd);
        return false;
    }
    if (!sender->isInChannel(chan->get_name()))
    {
        sender->send_message(":irc 442 " + sender->get_nick() + " " + chan->get_name() + " :You're not in that channel\r\n", fd);
        return false;
    }
    if (receiver->isInChannel(chan->get_name()))
    {
        sender->send_message(":irc 443 " + sender->get_nick() + " " + receiver->get_nick() + " " + chan->get_name() + " :is already on channel\r\n", fd);
        return false;
    }
    if (!chan->is_op(sender->get_nick()))
    {
        sender->send_message(":irc 482 " + sender->get_nick() + " " + chan->get_name() + " :You're not channel operator\r\n", fd);
        return false;
    }
    sender->send_message(":irc 341 " + sender->get_nick() + " " + receiver->get_nick() + " " + chan->get_name() + "\r\n", fd);
    send_to_channel(fd, chan->get_name(), ":" + sender->get_nick() + "!" + sender->get_user() + "@" + sender->get_hostname() + " INVITE " + receiver->get_nick() + " " + chan->get_name() + "\r\n");
    receiver->send_message(":" + sender->get_nick() + "!" + sender->get_user() + "@" + sender->get_hostname() + " INVITE " + receiver->get_nick() + " " + chan->get_name() + "\r\n", receiver->get_client_fd());
	chan->add_invite(receiver->get_nick());

	return (true);
}

void Client::remove_client_pointer(Channel *chan_to_remove)
{
    // Use std::remove_if with a lambda, or std::find followed by erase.
    // Using std::find is often simpler for a single element:
    std::vector<Channel *>::iterator it = _channels.begin();
    while (it != _channels.end()) {
		if (*it == chan_to_remove)
		{
			_channels.erase(it);
			return;
		}
		else
	        ++it;
    }
}

bool Server::part(int fd, std::vector<std::string> parts)
{
	if (parts.size() < 2)
	{
		_clients[fd]->send_message(":irc 461 " + _clients[fd]->get_nick() + " PART :Not enough parameters", fd);
		return false;
	}
	Client *sender = _clients[fd];
	std::string msg_reason;
	
	if (parts.size() >= 3)
	{
		if (parts[2][0] == ':')
			msg_reason = parts[2].substr(1);
	}
	else
		msg_reason = ":Leaving";
	std::vector<std::string> sp_channel = split(parts[1], ",");
	for (std::vector<std::string>::iterator it = sp_channel.begin(); it != sp_channel.end(); ++it)
	{
		Channel *chan = find_channel_name(*it);
		if (chan && sender->isInChannel(chan->get_name()))
		{
			sender->send_message(":" + sender->get_nick() + "!" + sender->get_user() + "@" + sender->get_hostname() + " PART " + chan->get_name() + " :" + msg_reason + "\r\n", fd);
			send_to_channel(fd, chan->get_name(), ":" + sender->get_nick() + "!" + sender->get_user() + "@" + sender->get_hostname() + " PART " + chan->get_name() + " :" + msg_reason + "\r\n");
			chan->remove_client(sender->get_nick(), *sender);
			sender->remove_client_pointer(chan);
		}
		else if (chan && !sender->isInChannel(chan->get_name()))
			sender->send_message(":irc 442 " + sender->get_nick() + " " + *it + " :You're not on that channel", fd);
		else
			sender->send_message(":irc 403 " + sender->get_nick() + " " + *it + " :No such channel", fd);
	}
	return (true);
}
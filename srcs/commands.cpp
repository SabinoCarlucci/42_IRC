/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   commands.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: negambar <negambar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/16 11:00:25 by scarlucc          #+#    #+#             */
/*   Updated: 2025/11/19 12:42:13 by negambar         ###   ########.fr       */
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
	(void)vect;
	std::string goodbye = "Goodbye\n";
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
		Channel	*new_channel = _channels[joinChannel];
		new_channel->add_clients(_clients[fd]->get_nick());
		_clients[fd]->add_client_pointer(new_channel);
		std::string full = ":" + sender->get_nick() + "!" + sender->get_user() + "@" + sender->get_hostname() + " JOIN :" + joinChannel + "\r\n";
		send_to_channel(fd, joinChannel, full, true);
		names(fd, parts);
	}
	else
	{
		Client *sender = _clients[fd];
    	if (sender->isInChannel(joinChannel) != NULL)
    	    return (true);
		receiver->add_clients(sender->get_nick()); 
		sender->add_client_pointer(receiver);
		std::string full = ":" + sender->get_nick() + "!" + sender->get_user() + "@" + sender->get_hostname() + " JOIN :" + joinChannel + "\r\n";
    	send_to_channel(fd, joinChannel, full, true);
    	std::string topic_reply = ":irc 332 " + sender->get_nick() + " " + joinChannel + " :The Topic of the Channel\r\n";
		sender->send_message(topic_reply, fd);
		
    	// 7. Send NAMES (353 and 366)
    	names(fd, parts);
	}
	return (true);
}

bool Server::mode(int fd, std::vector<std::string> parts)
{
	Client	*client = _clients[fd];
	if (parts.size() < 2)
	{
		client->send_message(":irc 461 " + client->get_nick() + " MODE :Not enough parameters", fd);
		delete	client;
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
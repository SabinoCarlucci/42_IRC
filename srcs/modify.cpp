#include "../includes/Channel.hpp"
#include "../includes/Server.hpp"
#include "../includes/Client.hpp"


void    Channel::modify_invite(Client &client, std::string params, bool what)
{
    (void)params;
    bool mod = _ops.find(client.get_nick()) != _ops.end();
    if (what && mod)
    {
        client.send_message(":" + client.get_nick() + "!" + client.get_user() + "@" + client.get_hostname() + " MODE " + _name + " +i\r\n", client.get_client_fd());
        serv->send_to_channel(client.get_client_fd(), this->get_name(), ":" + client.get_nick() + "!" + client.get_user() + "@" + client.get_hostname() + " MODE " + _name + " +i\r\n");
        _modes['i'] = 1;
    }
    else if (!what && mod)
    {
        client.send_message(":" + client.get_nick() + "!" + client.get_user() + "@" + client.get_hostname() + " MODE " + _name + " -i\r\n", client.get_client_fd());
        serv->send_to_channel(client.get_client_fd(), this->get_name(), ":" + client.get_nick() + "!" + client.get_user() + "@" + client.get_hostname() + " MODE " + _name + " -i\r\n");
        _modes['i'] = 0;
    }
    else
    {
        client.send_message(":irc 482 " + client.get_nick() + " " + _name + " :You're not a channel operator\r\n", client.get_client_fd());
    }
}

void    Channel::modify_topic(Client &c, std::string params, bool what)
{
    bool mod = _ops.find(c.get_nick()) != _ops.end();
    if (what && mod)
    {
        _modes['t'] = 1;
        if (!params.empty())
        {
            _topic = params;
            topuc(c, _topic);
        }
        c.send_message(":" + c.get_nick() + "!" + c.get_user() + "@" + c.get_hostname() + " MODE " + _name + " +t\r\n", c.get_client_fd());
        serv->send_to_channel(c.get_client_fd(), get_name(), ":" + c.get_nick() + "!" + c.get_user() + "@" + c.get_hostname() + " MODE " + _name + " +t\r\n");
    }
    else if (!what && mod)
    {
        _modes['t'] = 0;
        c.send_message(":" + c.get_nick() + "!" + c.get_user() + "@" + c.get_hostname() + " MODE " + _name + " -t\r\n", c.get_client_fd());
        serv->send_to_channel(c.get_client_fd(), get_name(), ":" + c.get_nick() + "!" + c.get_user() + "@" + c.get_hostname() + " MODE " + _name + " -t\r\n");
        if (params.find(" ") != std::string::npos)
            _topic = params.substr(params.find(" ") + 1);
    }
    else if (_modes['t'] == 0)
    {
        if (params.find(" ") != std::string::npos)
            _topic = params.substr(params.find(" ") + 1);
    }
    else
        c.send_message(":irc 482 " + c.get_nick() + " " + _name + " :You're not an operator of this channel", c.get_client_fd());
}


void Channel::modify_key(Client &c, std::string params, bool what)
{
    bool    mod = _ops.find(c.get_nick()) != _ops.end();
    if (what && mod)
    {
        _modes['k'] = 1;
        _pass = params;
        c.send_message(":" + c.get_nick() + "!" + c.get_user() + "@" + c.get_hostname() + " MODE " + _name + " +k " + params + "\r\n", c.get_client_fd());
        serv->send_to_channel(c.get_client_fd(), get_name(), ":" + c.get_nick() + "!" + c.get_user() + "@" + c.get_hostname() + " MODE " + _name + " +k " + params + "\r\n");
    }
    else if (!what && mod)
    {
        _modes['k'] = 0;
        c.send_message(":" + c.get_nick() + "!" + c.get_user() + "@" + c.get_hostname() + " MODE " + _name + " -k\r\n", c.get_client_fd());
        serv->send_to_channel(c.get_client_fd(), get_name(), ":" + c.get_nick() + "!" + c.get_user() + "@" + c.get_hostname() + " MODE " + _name + " -k\r\n");
        _pass = "";
    }
    else
    {
        c.send_message(":irc 482 " + c.get_nick() + " " + _name + " :You're not a channel operator", c.get_client_fd());
    }
}

void    Channel::modify_op(Client &c, std::string params, bool what)
{
    bool    mod = _ops.find(c.get_nick()) != _ops.end();
    if (what && mod)
    {
        if (!serv->find_by_nick(params)){
            c.send_message(":irc 401 " + c.get_nick() + " " + _name + " :No such nick", c.get_client_fd());
            return;
        }
        Client *add = serv->find_by_nick(params);
        if (std::find(_clients.begin(), _clients.end(), add->get_nick()) != _clients.end())
        {
            if (_ops.find(add->get_nick()) != _ops.end())
                return;
            if (_ops.size() >= 3)
            {
                c.send_message(":irc 400 " + c.get_nick() + " " + _name + " :Too many operators", c.get_client_fd());
            }
            _ops[add->get_nick()] = true;
            c.send_message(":" + c.get_nick() + "!" + c.get_user() + "@" + c.get_hostname() + " MODE " + _name + " +o " + params + "\r\n", c.get_client_fd());
            serv->send_to_channel(c.get_client_fd(), get_name(), ":" + c.get_nick() + "!" + c.get_user() + "@" + c.get_hostname() + " MODE " + _name + " +o " + params + "\r\n");
        }
        else
        {
            c.send_message(":irc 441 " + c.get_nick() + " " + params + " " + _name + " :They aren't on that channel\r\n", c.get_client_fd());
        }
    }
    else if (!what && mod)
    {
       if (_ops.find(params) != _ops.end())
       {
        c.send_message(":" + c.get_nick() + "!" + c.get_user() + "@" + c.get_hostname() + " MODE " + _name + " -o " + params + "\r\n", c.get_client_fd());
        serv->send_to_channel(c.get_client_fd(), get_name(), ":" + c.get_nick() + "!" + c.get_user() + "@" + c.get_hostname() + " MODE " + _name + " -o " + params + "\r\n");
        _ops.erase(params);
        Client *de_opped_client = serv->find_by_nick(params);
        if (de_opped_client)
             de_opped_client->send_message(":" + c.get_nick() + "!" + c.get_user() + "@" + c.get_hostname() + " MODE " + _name + " -o " + params + "\r\n", de_opped_client->get_client_fd());
        }
        else
        {
            c.send_message(":irc 441 " + c.get_nick() + " " + _name + " " + params + " :They aren't a channel operator\r\n", c.get_client_fd());
        }
    }
    else
    {
        c.send_message(":irc 482 " + c.get_nick() + " " + _name + " :You're not a channel operator", c.get_client_fd());
    }
}

void Channel::modify_limit(Client &c, std::string params, bool what)
{
    bool mod = _ops.find(c.get_nick()) != _ops.end();
    
    if (!mod)
    {
        c.send_message(":irc 482 " + c.get_nick() + " " + _name + " :You're not a channel operator", c.get_client_fd());
        return;
    }
    if (what)
    {
        if (params.empty()) {
            c.send_message(":irc 696 " + c.get_nick() + " " + _name + " l :You must specify a parameter for the limit mode", c.get_client_fd());
            return;
        }
        
        char *end;
        long limit_val = std::strtol(params.c_str(), &end, 10);
        if (*end != '\0' || limit_val <= 0)
        {
            c.send_message(":irc 400 " + c.get_nick() + " " + _name + " :Invalid limit", c.get_client_fd());
            return;
        }
        
        _modes['l'] = static_cast<int>(limit_val);
        c.send_message(":" + c.get_nick() + "!" + c.get_user() + "@" + c.get_hostname() + " MODE " + _name + " +l " + params + "\r\n", c.get_client_fd());
        serv->send_to_channel(c.get_client_fd(), get_name(), ":" + c.get_nick() + "!" + c.get_user() + "@" + c.get_hostname() + " MODE " + _name + " +l " + params + "\r\n");
    }
    else
    {
        _modes['l'] = 0;
        c.send_message(":" + c.get_nick() + "!" + c.get_user() + "@" + c.get_hostname() + " MODE " + _name + " -l\r\n", c.get_client_fd());
        serv->send_to_channel(c.get_client_fd(), get_name(), ":" + c.get_nick() + "!" + c.get_user() + "@" + c.get_hostname() + " MODE " + _name + " -l\r\n");
    }
}
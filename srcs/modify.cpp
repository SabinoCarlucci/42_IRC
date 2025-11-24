#include "../includes/Channel.hpp"
#include "../includes/Server.hpp"
#include "../includes/Client.hpp"


void    Channel::modify_invite(Client &client, std::string params, bool what)
{
    (void)params;
    bool mod = _ops.find(client.get_nick()) != _ops.end();
    if (what && mod)
    {
        std::cout << "here it works" << std::endl;
        serv->send_to_channel(client.get_client_fd(), this->get_name(), ":" + client.get_nick() + "!" + client.get_user() + "@" + client.get_hostname() + " MODE " + _name + "+i\r\n", true);
        _modes['i'] = 1;
    }
    else if (!what && mod)
    {
        serv->send_to_channel(client.get_client_fd(), this->get_name(), ":" + client.get_nick() + "!" + client.get_user() + "@" + client.get_hostname() + " MODE " + _name + "-i\r\n", true);
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
        serv->send_to_channel(c.get_client_fd(), get_name(), ":" + c.get_nick() + "!" + c.get_user() + "@" + c.get_hostname() + " MODE " + _name + " +t\r\n", true);
    }
    else if (!what && mod)
    {
        _modes['t'] = 0;
        serv->send_to_channel(c.get_client_fd(), get_name(), ":" + c.get_nick() + "!" + c.get_user() + "@" + c.get_hostname() + " MODE " + _name + " -t\r\n", true);
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
#include "../includes/Channel.hpp"
#include "../includes/Server.hpp"
#include "../includes/Client.hpp"

Channel::Channel(std::string name, Server *server)  : serv(server) 
{
	_name = name;
	_modes['i'] = false;
	_modes['t'] = false;

    _modes['o'] = false;
	_modes['l'] = false;
	_modes['b'] = false;


 	_mode_funcs['i'] = &Channel::modify_invite;
	_mode_funcs['t'] = &Channel::modify_topic;
	_mode_funcs['k'] = &Channel::modify_key;
	_mode_funcs['o'] = &Channel::modify_op;
	_mode_funcs['l'] = &Channel::modify_limit;
}

std::string Channel::get_name() const
{
    return (_name);
}

std::string Channel::get_pass() const
{
    return (_pass);
}

bool Channel::send_message(std::string message, int fd)
{
	message += "\r\n";
	if (send(fd, message.c_str(), message.size(), 0) == -1)
	{
		return false;
	}
	return true;
}

const std::vector<std::string> &Channel::get_clients() const
{
	return (_clients);
}

bool    Channel::is_op(std::string client)
{
    if (_ops.find(client) != _ops.end())
        return (true);
    return (false);
}

void	Channel::send_modes(Client &client, int fd)
{
	std::string start = ":irc 324 " + client.get_nick() + " " + get_name();
	std::string modes = " +";
	std::string key = "";
	std::string limit = "";
	for (std::map<char, bool>::iterator it = _modes.begin(); it != _modes.end(); ++it)
	{
		if (it->second)
			modes += it->first;
	}
	if (_modes['l'])
		limit += " " + NumberToString(_modes['l']);
	if (_modes['k'])
		limit += " " + _pass;
	if (modes == " +")
		modes = "none";
	if (modes.find('l') < modes.find('k'))
		client.send_message(start + " " + modes + limit + key, fd);
	else
		client.send_message(start + " " + modes + key + limit, fd);
}

bool Channel::modify_mode(std::vector<std::string> parts, Client &client, int fd)
{
    // Mode string is parts[2], Arguments start from parts[3]
    if (parts.size() < 3) {
        // Not enough parts to even have a mode string
        return false;
    }

    size_t arg_index = 3;
    char sign = '+';

    for (size_t i = 0; i < parts[2].length(); ++i)
    {
        char mode_char = parts[2][i];
        std::string mode_arg = "";

        if (mode_char == '+' || mode_char == '-')
        {
            sign = mode_char;
            continue;
        }
        if (_mode_funcs.find(mode_char) == _mode_funcs.end())
        {
            client.send_message(":irc 472 " + client.get_nick() + " " + _name + " " + mode_char + " :is unknown mode char to me", fd);
            continue; 
        }

        bool needs_arg = (mode_char == 'o' || mode_char == 'k' || mode_char == 'l'); 
        
        if (needs_arg && arg_index < parts.size())
        {
             mode_arg = parts[arg_index];
             arg_index++; // Consume the argument
        }
        // If it needs an arg and we don't have one, your specific mode function should handle the error (461 ERR_NEEDMOREPARAMS).

        // 4. Call the mode function
        bool is_set = (sign == '+');
        (this->*_mode_funcs[mode_char])(client, mode_arg, is_set);
    }
    
    return true;
}

void Server::remove_channel(std::string name)
{
    if (_channels.find(name) != _channels.end())
    {
        delete _channels[name];
        _channels.erase(name);
    }
}

void Channel::remove_client(std::string nick, Client &c)
{
    std::vector<std::string>::iterator it = std::find(_clients.begin(), _clients.end(), nick);
    if (it != _clients.end())
        _clients.erase(it);
    if (_ops.find(nick) != _ops.end())
        _ops.erase(nick);
    if (this->size() == 0)
    {
        serv->remove_channel(_name);
    }
    else if (_ops.size() == 0 && !_clients.empty())
    {
        try
        {
            std::vector<std::string>::iterator it = _clients.begin();
            std::advance(it, static_cast<size_t>(rand()) % _clients.size());
            _ops[*it] = true;
            serv->send_to_channel(c.get_client_fd(), get_name(), ":" + c.get_nick() + "!" + c.get_user() + "@" + c.get_hostname() + " MODE " + _name + " +o " + *it + "\r\n");
        }
        catch(const std::exception &e)
        {
            std::cerr << e.what() << "\n";
        }
    }
}
void	Channel::send_to_all( std::string quit_msg )
{
	for (std::vector<std::string>::iterator client = _clients.begin(); client != _clients.end(); client++)
	{
		Client *this_client = serv->find_by_nick(*client);
		int fd = this_client->get_client_fd();
		send_message(quit_msg, fd);
	}
}


void	Channel::quit_user( std::string user, std::string quit_msg )
{
    Client *c = serv->find_by_nick(user);
	this->remove_user(user);
	for (std::vector<std::string>::iterator client = _clients.begin(); client != _clients.end(); client++)
	{
		Client *this_client = serv->find_by_nick(*client);
		int fd = this_client->get_client_fd();
		send_message(quit_msg, fd);
	}
    try
    {
        std::vector<std::string>::iterator it = _clients.begin();
        std::advance(it, static_cast<size_t>(rand()) % _clients.size());
        _ops[*it] = true;
        serv->send_to_channel(c->get_client_fd(), get_name(), ":" + c->get_nick() + "!" + c->get_user() + "@" + c->get_hostname() + " MODE " + _name + " +o " + *it + "\r\n");
    }
    catch(const std::exception &e)
    {
        std::cerr << e.what() << "\n";
    }
}

void	Channel::change_nick_user( std::string user, std::string msg )
{
	
}
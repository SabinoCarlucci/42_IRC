#include "../includes/Channel.hpp"
#include "../includes/Server.hpp"
#include "../includes/Client.hpp"

Channel::Channel(std::string name/* , Server *server */)/*  : serv(server) */
{
	_name = name;
	_modes['i'] = false;
	_modes['t'] = false;
	_modes['k'] = false;
	_modes['o'] = false;
	_modes['l'] = false;
	_modes['b'] = false;


/* 	_mode_funcs['i'] = &Channel::modify_invite;
	_mode_funcs['t'] = &Channel::modify_topic;
	_mode_funcs['k'] = &Channel::modify_key;
	_mode_funcs['o'] = &Channel::modify_op;
	_mode_funcs['l'] = &Channel::modify_limit;
	_mode_funcs['b'] = &Channel::modify_ban; */
}

std::string Channel::get_name()
{
    return (_name);
}

std::string Channel::get_pass()
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

std::vector<std::string> Channel::get_clients()
{
	return (_clients);
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

    // parts[3] is the first argument, parts[4] is the second, etc.
    size_t arg_index = 3;
    char sign = '+'; // Start assuming '+' if the first char is a mode letter

    // parts[2] is the mode string, e.g., "+ok-i"
    for (size_t i = 0; i < parts[2].length(); ++i)
    {
        char mode_char = parts[2][i];
        std::string mode_arg = "";
        
        // 1. Check for sign change
        if (mode_char == '+' || mode_char == '-')
        {
            sign = mode_char;
            continue; // Move to the next character (which should be the mode letter)
        }
        
        // At this point, mode_char is a mode letter (e.g., 'o', 'k', 'i')
        
        // 2. Check if the mode exists
        if (_mode_funcs.find(mode_char) == _mode_funcs.end())
        {
            // Unknown mode
            client.send_message(":irc 472 " + client.get_nick() + " " + _name + " " + mode_char + " :is unknown mode char to me", fd);
            // It's usually okay to continue parsing other modes, but for simplicity, we return true if we sent a response.
            continue; 
        }

        // 3. Check for argument requirement (This assumes your mode functions know which modes take args)
        // **NOTE:** This is the most complex part of IRC MODE. Modes 'o', 'k', 'l', 'b', 'v', 'a', 'i', 't', 'n', 'm', 'p', 's', 'r' 
        // have different rules for args on add/remove, which you must check inside the mode functions.
        // For simplicity, we just check if an argument is available and pass it if required.
        // A common simplification is to assume modes 'o' and 'k' always require an argument on ADD and sometimes on REMOVE.
        
        // For simplicity in this fix, we will just pass the next available argument if there is one.
        // Your mode function needs to check if it's the correct mode and argument is needed.
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
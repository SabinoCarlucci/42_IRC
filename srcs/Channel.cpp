#include "../includes/Channel.hpp"
#include "../includes/Server.hpp"
#include "../includes/Client.hpp"


Channel::Channel(std::string name)
{
	_name = name;
	_modes["invite"] = false;
	_modes["topic"] = false;
	_modes["key"] = false;
	_modes["operator"] = false;
	_modes["limit"] = false;
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


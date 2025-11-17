#include "../includes/Channel.hpp"
#include "../includes/Server.hpp"
#include "../includes/Client.hpp"


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
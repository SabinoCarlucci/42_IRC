#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <vector>
#include <map>
#include <poll.h>
#include <stdio.h>
#include <errno.h>
#include <sstream>

class Client;
class Server;

class Channel
{
    private:
        std::string _name;
        std::map<std::string, bool> _restricts; //tipo "invite only" = false/true
        std::string _pass;
        std::vector<std::string> _clients;
    public:
        std::string                 get_name();
        std::string                 get_pass();
        std::vector<std::string>    get_clients();
        bool                        send_message(std::string message, int fd);
};


#endif
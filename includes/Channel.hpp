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
        std::map<std::string, bool> _modes; //tipo "invite only" = false/true
        std::string _pass;
        std::vector<std::string> _clients;
    public:
        Channel(std::string name);
        std::string                 get_name();
        std::string                 get_pass();
        std::vector<std::string>    get_clients();
        void                        add_clients(std::string name) {_clients.push_back(name); };
        bool                        send_message(std::string message, int fd);
};


#endif
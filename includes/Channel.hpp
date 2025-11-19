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
		// Server		*serv;
		std::string			_topic;
		std::map<std::string, bool> _ops;
		std::vector<std::string> _bans;
        std::map<char, bool> _modes; //tipo "invite only" = false/true
        std::string _pass;
        std::vector<std::string> _clients;
        std::map<char, void (Channel::*)(Client&, std::string, bool)> _mode_funcs;    public:
    public:
		Channel(std::string name/* , Server *server */);
        std::string                 get_name();
        std::string                 get_pass();
        std::vector<std::string>    get_clients();
        void                        send_modes(Client &client, int fd);
        void                        add_clients(std::string name) {_clients.push_back(name); };
        bool                        send_message(std::string message, int fd);
		
 		bool						modify_mode(std::vector<std::string> parts, Client &client, int fd);

/*		void						modify_invite(Client &client, std::string params, bool what);
		void						modify_topic(Client &client, std::string params, bool what);
		void						modify_key(Client &client, std::string params, bool what);
		void						modify_ban(Client &client, std::string params, bool what);
		void						modify_op(Client &client, std::string params, bool what);
		void						modify_limit(Client &client, std::string params, bool what);
		
		void						send_bans(Client &client);
		void						topuc(Client &client, std::string parameters);
		bool						is_ban(std::string client) const; */
};

template <typename T>
std::string NumberToString(T Number)
{
	std::ostringstream ss;
	ss << Number;
	return ss.str();
};

inline std::ostream &operator<<(std::ostream &o, const std::vector<std::string> &v)
{
	o << "[";
	for (size_t i = 0; i < v.size(); i++)
	{
		o << v[i];
		if (i + 1 < v.size())
			o << ", ";
	}
	o << "]";
	return o;
}


///Prints all the cannels and their clients
inline std::ostream &operator<<(std::ostream &o, std::map<std::string, Channel*> const &v) {
	for (std::map<std::string, Channel*>::const_iterator it = v.begin(); it != v.end(); ++it)
		o << "Channel: "<<it->first << " Clients: " << it->second->get_clients() << std::endl;

	o << std::endl;
	return o;
}

///Prints all the modes of a channel
inline std::ostream &operator<<(std::ostream &o, std::map<char, void (Channel::*)(Client&, std::string, bool)> const &v) {
	for (std::map<char, void (Channel::*)(Client&, std::string, bool)>::const_iterator it = v.begin(); it != v.end(); ++it)
		o << "\""<<it->first << "\" ";

	o << std::endl;
	return o;
}

#endif
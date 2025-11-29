#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <vector>
#include <map>
#include <poll.h>
#include <stdio.h>
#include <errno.h>
#include <sstream>
#include <algorithm>

class Client;
class Server;

class Channel
{
    private:
        std::string _name;
		Server		*serv;
		int			limit;
		std::string			_topic;
		std::map<std::string, bool> _ops;
        std::map<char, bool> _modes; //tipo "invite only" = false/true
        std::string _pass;
        std::vector<std::string> _clients;
		std::vector<std::string> _invites;
        std::map<char, void (Channel::*)(Client&, std::string, bool)> _mode_funcs;
    public:
		Channel(std::string name, Server *server);
        std::string                 get_name() const;
        std::string                 get_pass() const;


		void						remove_client(std::string nick, Client &c);
		bool						is_op(std::string client);
		bool						is_invited(std::string nick);

		size_t						size() const { return(_clients.size() + _ops.size());}
		const std::vector<std::string>	&get_clients() const;
		void						add_op(std::string op) { _ops[op]; }
        void                        send_modes(Client &client, int fd);
        void                        add_clients(std::string name) {_clients.push_back(name); };
        bool                        send_message(std::string message, int fd);
		void						add_invite(const std::string &nick) {_invites.push_back(nick);}
		void						remove_invite(const std::string &nick) {std::remove(_invites.begin(), _invites.end(), nick), _invites.end();}

		bool						modify_mode(std::vector<std::string> parts, Client &client, int fd);

		void						modify_invite(Client &client, std::string params, bool what);
		void						modify_topic(Client &client, std::string params, bool what);
		void						modify_key(Client &client, std::string params, bool what);
		void						modify_op(Client &client, std::string params, bool what);
		void						modify_limit(Client &client, std::string params, bool what);
		void						topuc(Client &client, std::string parameters);
		void						join_channel(Client &c, std::vector<std::string> parts, int fd); //cambiato prototipo join_channel per evitare doppio messaggio di entrata
		void						send_to_all( std::string quit_msg ); //versione semplificata di send_to_channel
		void						remove_user( std::string user ) { this->_clients.erase(std::find(this->_clients.begin(),this->_clients.end(), user));}
		void						quit_user( std::string user, std::string quit_msg );
		void						change_nick_user( std::string user, std::string msg );
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
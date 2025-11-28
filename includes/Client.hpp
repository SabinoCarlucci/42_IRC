/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: scarlucc <scarlucc@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/15 11:42:06 by scarlucc          #+#    #+#             */
/*   Updated: 2025/11/26 18:41:56 by scarlucc         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT
#define CLIENT

#include <string>
#include <netinet/in.h>
#include <string>
#include <vector>
#include <map>
#include <poll.h>
#include <stdio.h>
#include <errno.h>
#include <set>
#include <algorithm>
#include <sstream>

class Channel;

class Client
{
private:
    int 		_fd;
    std::string _buffer;
    std::string _nick;
    std::string _user;
    std::string _hostname;
	bool		_authenticated;
    std::vector<Channel *> _channels;
	
public:
    Client(int fd);
    ~Client();

    std::string &buffer();
	
	int get_client_fd() const;
	//void set_client_fd(int fd);

    std::string &get_hostname();
    void set_hostname(const std::string &h);
    
    bool get_authenticated() const;
    void set_authenticated();
	
    std::string get_nick() const;
	void set_nick(const std::string &n);
	
    std::string get_user() const;
	void set_user(const std::string &u);

    bool send_message(std::string message, int fd);
    Channel *isInChannel(std::string name);
	std::vector<Channel*>& getChannels();
	const std::vector<Channel*>& getChannels() const;


    void    add_client_pointer(Channel *channel);
};

//prints alle the information about the client
std::ostream &operator<<(std::ostream &out, Client const &src);

//prints all the client's nick in a vector
inline std::ostream &operator<<(std::ostream &out, std::vector<Client*> clients) {
	for (std::vector<Client*>::iterator it = clients.begin(); it != clients.end(); ++it)
	{
		out << " ["<<(*it)->get_nick() << "] ";
	}
	return out;
}

//prints all the clients in a map
inline std::ostream &operator<<(std::ostream &o, std::map<std::string, Client*> const &v) {
	for (std::map<std::string, Client*>::const_iterator it = v.begin(); it != v.end(); ++it)
		o << it->first;

	o << std::endl;
	return o;
}

//prints all the clients in a set
inline std::ostream &operator<<(std::ostream &o, std::set<std::string> const &v) {
	for (std::set<std::string>::const_iterator it = v.begin(); it != v.end(); ++it)
		o <<"["<< *it <<"]";
		
	o << std::endl;
	return o;
}


#endif
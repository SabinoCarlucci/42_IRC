/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: negambar <negambar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/15 11:42:06 by scarlucc          #+#    #+#             */
/*   Updated: 2025/11/16 11:46:29 by negambar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT
#define CLIENT

#include <string>
#include <netinet/in.h>

class Client
{
private:
    int 		_fd;
    std::string _buffer;
    std::string _nick;
    std::string _user;
    std::string _hostname;
	bool		_authenticated;
	
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
};

#endif
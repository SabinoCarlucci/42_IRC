/******************************************************************************/
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: scarlucc <scarlucc@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/15 11:42:06 by scarlucc          #+#    #+#             */
/*   Updated: 2025/11/15 12:36:54 by scarlucc         ###   ########.fr       */
/*                                                                            */
/******************************************************************************/

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
	bool		_authenticated;
	
public:
    Client(int fd);
    ~Client();

    std::string &buffer();
	
	int get_client_fd() const;
	//void set_client_fd(int fd);
    
    bool get_authenticated() const;
    void set_authenticated();
	
    std::string get_nick() const;
	void set_nick(const std::string &n);
	
    std::string get_user() const;
	void set_user(const std::string &u);
};

#endif
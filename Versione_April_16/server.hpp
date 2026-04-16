/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cacorrea <cacorrea@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/13 14:04:19 by sel-khao          #+#    #+#             */
/*   Updated: 2026/04/16 16:39:53 by cacorrea         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include "client.hpp"

#include <iostream>
#include <cstdlib>
#include <poll.h>
#include <sys/time.h>
#include <vector>
#include <cerrno>
#include <sys/unistd.h>
#include <sys/fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h> 
#include <string>
#include <unistd.h>
#include <fcntl.h>

class Server
{
	private:
		std::vector<struct pollfd>	poll_fds;
		std::vector<Client>			client_vect;
		int							socket_fd;
		std::string					server_port;
		std::string					server_password;

		void initServer();                // chiama setupServer
		int setupServer(int port);        // privato
		void acceptNewClient();           // usa i membri
		bool handleClientRead(size_t i);  // i = indice in poll_fds
		void handleClientWrite();         // itera su poll_fds
		void sendToClient(int client_fd, const std::string& message);
	public:
		Server(const std::string& port, const std::string& password);
		~Server();
		void run();
};



#endif
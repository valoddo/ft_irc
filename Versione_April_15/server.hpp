/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sel-khao <sel-khao@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/13 14:04:19 by sel-khao          #+#    #+#             */
/*   Updated: 2026/04/15 18:19:22 by sel-khao         ###   ########.fr       */
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

class Server{
    private:
        

    public:
        
};


void handleClientWrite(std::vector <struct pollfd> poll_fds, std::vector <Client> client_vect);
bool handleClientRead(unsigned long i, std::vector<struct pollfd>& poll_fds, std::vector<Client>& client_vect);
int acceptNewClient(int socket_fd, struct sockaddr_in &client_addr, std::vector <struct pollfd> &poll_fds, std::vector <Client> &client_vect);
int setupServer(int port);
void sendToClient(int client_fd, const std::string& message, std::vector<Client>& clients, std::vector<struct pollfd>& poll_fds);

#endif
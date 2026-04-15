/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sel-khao <sel-khao@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/13 14:04:07 by sel-khao          #+#    #+#             */
/*   Updated: 2026/04/15 18:18:16 by sel-khao         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.hpp"
#include "client.hpp"

int main(int argc, char **argv)
{    
    if (argc != 3){
        std::cout << "wrong input: ./program, argv[1] port, argv[2] password" << std::endl;
        return 1;
    }
    int port = std::atoi(argv[1]);
    int socket_fd = setupServer(port);
    struct sockaddr_in client_addr;
    std::vector <struct pollfd> poll_fds;
    struct pollfd server_pollfd;
    server_pollfd.fd = socket_fd;
    server_pollfd.events = POLLIN;
    poll_fds.push_back(server_pollfd);
    std::vector <Client> client_vect;
    client_vect.push_back(Client());
    for (;;){
        if (poll(&poll_fds[0], poll_fds.size(), -1) == -1){
            std::cout << "poll failed" << std::endl;
            return 1;
        }
        if (poll_fds[0].revents == POLLIN)
			acceptNewClient(socket_fd, client_addr, poll_fds, client_vect);
        for (unsigned long i = 1; i < poll_fds.size(); i++){
            if (poll_fds[i].revents == POLLIN){
                handleClientRead(i, poll_fds, client_vect);
            }
        }
		handleClientWrite(poll_fds, client_vect);
    }
}

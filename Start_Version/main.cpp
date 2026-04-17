/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sel-khao <sel-khao@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/13 14:04:07 by sel-khao          #+#    #+#             */
/*   Updated: 2026/04/14 17:59:48 by sel-khao         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.hpp"
#include <cstring>

int main(int argc, char **argv)
{
    if (argc != 3){
        std::cout << "wrong input: ./program, argv[1] port, argv[2] password" << std::endl;
        return 1;
    }
    int port = std::atoi(argv[1]);
    //int socket(int __domain, int __type, int __protocol)
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);//gimmee an IPv4, TCP socket, u(os) pick exact protocol
    if (socket_fd == -1){
        std::cout << "socket failed" << std::endl;
        return 1;
    }
    struct sockaddr_in address;//IPv4
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(port);//Convert multi-byte integer types from host byte order to network byte order //network to host short
    address.sin_addr.s_addr = INADDR_ANY;//ascolta su tutte le interfacce di rete del server(localhost, wifi, ethernet...)
    //int bind(int sockfd, struct sockaddr *my_addr, int addrlen);
    if (bind(socket_fd, (struct sockaddr*)&address, sizeof(address)) == -1){
        std::cout << "bind failed" << std::endl;
        return 1;
    }
    //int listen(int sockfd, int backlog);
    //ncoming connections are going to wait in this queue until you accept(), silent limit is 20
    if (listen(socket_fd, 10) == -1){
        std::cout << "listen failed" << std::endl;
        return 1;
    }
    if (fcntl(socket_fd, F_SETFL, O_NONBLOCK) == -1){
        std::cout << "blocking fd didnt succeed" << std::endl;
        return 1;
    }
    //int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
    struct sockaddr_storage client_addr;//modulo con info of client, ip and port ecc..
    std::vector <struct pollfd> poll_fds;//creao un vettore vuoto di nome fds
    //ora crea variabile di tipo struct pollfd per il server
    struct pollfd server_pollfd;
    server_pollfd.fd = socket_fd;//server socket
    server_pollfd.events = POLLIN;
    //int nfds = 1;//numero di slot attualmente in uso, 1 socket server e' attivo
    poll_fds.push_back(server_pollfd);
    for (;;){
        if (poll(&poll_fds[0], poll_fds.size(), -1) == -1){//ora controlla cosa e' successo
            std::cout << "poll failed" << std::endl;
            return 1;
        }
        if (poll_fds[0].revents == POLLIN){
            socklen_t addr_size = sizeof(client_addr);//size of it
            int client_socket = accept(socket_fd, (struct sockaddr*)&client_addr, &addr_size);
            if (client_socket == -1){
                std::cout << "accept failed" << std::endl;
                return 1;
            }
            if (fcntl(client_socket, F_SETFL, O_NONBLOCK) == -1){
                std::cout << "blocking fd didnt succeed" << std::endl;
                return 1;
            }
            struct pollfd client_pollfd;//ogni client q voglio monitorare deve avere un arrayy di struct pollfd
            client_pollfd.fd = client_socket;//quale socket controllare?
            client_pollfd.events = POLLIN;//POLLIN dice a poll() di avvertire quando ha dati da leggere. gotta know when a client sends sms
            poll_fds.push_back(client_pollfd);
        }
        for (int i = 1; i < poll_fds.size(); i++){
            if (poll_fds[i].revents == POLLIN){
                char buffer[512];
                ssize_t bytes_letti;
                bytes_letti = recv(poll_fds[i].fd, buffer, 512, 0);
                if (bytes_letti > 0){
                    buffer[bytes_letti] = '\0';
                    std::cout << "ricevuto da client " << i << ": " << buffer << std::endl;
                }
                else if (bytes_letti == 0){//client disconnesso
                    close(poll_fds[i].fd);
                    poll_fds.erase(poll_fds.begin() + i);
                    i--;
                    continue ;
                }
                else{
                    if (errno != EAGAIN && errno != EWOULDBLOCK){
                        close(poll_fds[i].fd);
                        poll_fds.erase(poll_fds.begin() + i);
                        i--;
                        continue ;
                    }
                }
            }
        }
    }
}


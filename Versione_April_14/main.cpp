/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vloddo <vloddo@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/13 14:04:07 by sel-khao          #+#    #+#             */
/*   Updated: 2026/04/14 21:47:50 by vloddo           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.hpp"
#include "client.hpp"
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
    if (listen(socket_fd, 10) == -1){ // qui viene creata la backlog, lista che passa per il poll
        std::cout << "listen failed" << std::endl;
        return 1;
    }
    
    if (fcntl(socket_fd, F_SETFL, O_NONBLOCK) == -1){
        std::cout << "blocking fd didnt succeed" << std::endl;
        return 1;
    }
    //int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
    //struct sockaddr_storage client_addr;//modulo con info of client, ip and port ecc..
    struct sockaddr_in client_addr;// cambiato in sockaddr_in per tenere IPv4
    std::vector <struct pollfd> poll_fds;//creao un vettore vuoto di nome fds
    struct pollfd server_pollfd; //ora crea variabile di tipo struct pollfd per il server
    server_pollfd.fd = socket_fd;//server socket
    server_pollfd.events = POLLIN;
    //int nfds = 1;//numero di slot attualmente in uso, 1 socket server e' attivo
    poll_fds.push_back(server_pollfd); // Placeholder per indice 0 (fd del server), gli altri fd sono client1, client2 ecc..
    
    std::vector <Client> client_vect; // creao un vettore di Clients
    client_vect.push_back(Client());  // Placeholder per indice 0 (in modo da restare in linea con gli indici del fd server)
    for (;;){
        if (poll(&poll_fds[0], poll_fds.size(), -1) == -1){//ora controlla cosa e' successo
            std::cout << "poll failed" << std::endl;
            return 1;
        }
        if (poll_fds[0].revents == POLLIN){ // sul server socket c'e nuova connessione nella coda del backlog (dopo aver completato il three-way handshake TCP)
            socklen_t addr_size = sizeof(client_addr);//size of it
            int client_socket = accept(socket_fd, (struct sockaddr*)&client_addr, &addr_size); // cast con sockaddr necessario perche accept prende solo questa struttura
            if (client_socket == -1){
                std::cout << "accept failed" << std::endl;
                return 1;
            } 
                     if (fcntl(client_socket, F_SETFL, O_NONBLOCK) == -1){
                std::cout << "blocking fd didnt succeed" << std::endl;
                return 1;
            }
            Client client;
            client.setFd(client_socket);
            char clientIP[INET_ADDRSTRLEN];  // Array con memoria allocata
            inet_ntop(AF_INET, &client_addr.sin_addr, clientIP, INET_ADDRSTRLEN); //la funzione converte in IPv4 ma e una funzione di c ed usa char
            client.setIP(std::string(clientIP));
            client_vect.push_back(client);
   
            struct pollfd client_pollfd;//ogni client q voglio monitorare deve avere un arrayy di struct pollfd
            client_pollfd.fd = client_socket;//quale socket controllare?
            client_pollfd.events = POLLIN;//POLLIN dice a poll() di avvertire quando ha dati da leggere. gotta know when a client sends sms
            poll_fds.push_back(client_pollfd);
        }
        for (unsigned long i = 1; i < poll_fds.size(); i++){
            if (poll_fds[i].revents == POLLIN){
                char buffer[512];
                ssize_t bytes_letti;
                bytes_letti = recv(poll_fds[i].fd, buffer, 512, 0);
                if (bytes_letti > 0){
                    buffer[bytes_letti] = '\0';
                    std::cout << "ricevuto da client " << i << ": " << buffer << std::endl;
                    client_vect[i].getBuffer() += buffer;

// Parsing semplificato per Test

                    std::string& buf = client_vect[i].getBuffer();
                    size_t pos;

                    while ((pos = buf.find("\r\n")) != std::string::npos) {
                        std::string line = buf.substr(0, pos);
                        buf.erase(0, pos + 2);
                        
                        std::cout << "Comando: " << line << std::endl;
                        
                        // PING
                        if (line.find("PING") == 0) {
                            std::string pong = "PONG" + line.substr(4) + "\r\n";
                            send(client_vect[i].getFd(), pong.c_str(), pong.size(), 0);
                            std::cout << "Inviato: " << pong;
                        }
                        
                        // NICK
                        else if (line.find("NICK") == 0) {
                            std::string nick = line.substr(5);
                            // togli spazi
                            nick.erase(0, nick.find_first_not_of(" \t"));
                            nick.erase(nick.find_last_not_of(" \t") + 1);
                            client_vect[i].setNick(nick);
                            std::cout << "Nick impostato: " << nick << std::endl;
                        }
                        
                        // USER
                        else if (line.find("USER") == 0) {
                            std::string rest = line.substr(5);
                            size_t space = rest.find(' ');
                            if (space != std::string::npos) {
                                std::string username = rest.substr(0, space);
                                client_vect[i].setUser(username);
                                std::cout << "User impostato: " << username << std::endl;
                            }
                        }
                        
                        // Welcome message (dopo NICK e USER)
                        if (!client_vect[i].getNick().empty() && 
                            !client_vect[i].getUser().empty() && 
                            !client_vect[i].isAuthenticated()) {
                            
                            client_vect[i].setAuthenticated(true);
                            
                            std::string welcome = 
                                ":server 001 " + client_vect[i].getNick() + " :Welcome to IRC\r\n" +
                                ":server 002 " + client_vect[i].getNick() + " :Your host is server\r\n" +
                                ":server 003 " + client_vect[i].getNick() + " :Server created\r\n" +
                                ":server 004 " + client_vect[i].getNick() + " :server IRC\r\n";
                            
                            send(client_vect[i].getFd(), welcome.c_str(), welcome.size(), 0);
                            std::cout << "Welcome inviato a " << client_vect[i].getNick() << std::endl;
                        }
                    }

//______________________________________________________________________________-
                    
                }
                else if (bytes_letti == 0){//client disconnesso
                    close(poll_fds[i].fd);
                    poll_fds.erase(poll_fds.begin() + i);
                    client_vect.erase(client_vect.begin() + i);
                    i--;
                    continue ;
                }
                else{
                    if (errno != EAGAIN && errno != EWOULDBLOCK){ // gli errori EAGAIN e EWOULDBLOCK sono errori che indicano che non ci sono dati da leggere al momento
                        close(poll_fds[i].fd);
                        poll_fds.erase(poll_fds.begin() + i);
                        client_vect.erase(client_vect.begin() + i);
                        i--;
                        continue ;
                    }
                }
            }
        }
    }
}


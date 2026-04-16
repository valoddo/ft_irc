/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: marvin <marvin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/13 14:04:12 by sel-khao          #+#    #+#             */
/*   Updated: 2026/04/16 02:05:46 by marvin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.hpp"

Server::Server(const std::string& port, const std::string& password)
{
    server_port = port;
    server_password = password;
    socket_fd = -1;
    int port_int = std::atoi(port.c_str()); // Inizializza il server (chiama setupServer)
    if (setupServer(port_int) == -1)
    {
        std::cerr << "Failed to initialize server" << std::endl;
        return;
    }
    struct pollfd server_pollfd;
    server_pollfd.fd = socket_fd;
    server_pollfd.events = POLLIN;
    poll_fds.push_back(server_pollfd);
}

Server::~Server()
{
    if (socket_fd != -1)
        close(socket_fd);
    for (size_t i = 0; i < poll_fds.size(); i++)
        close(poll_fds[i].fd);
}

int Server::setupServer(int port)
{
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1)
    {
        std::cout << "socket failed" << std::endl;
        return -1;
    }
    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = INADDR_ANY;
    if (bind(socket_fd, (struct sockaddr*)&address, sizeof(address)) == -1)
    {
        std::cout << "bind failed" << std::endl;
        return -1;
    }
    if (listen(socket_fd, 10) == -1)
    {
        std::cout << "listen failed" << std::endl;
        return -1;
    }
    if (fcntl(socket_fd, F_SETFL, O_NONBLOCK) == -1)
    {
        std::cout << "blocking fd didnt succeed" << std::endl;
        return -1;
    }
    return socket_fd;
}

void Server::acceptNewClient()
{
    struct sockaddr_in client_addr;
    socklen_t addr_size = sizeof(client_addr);
    int client_socket = accept(socket_fd, (struct sockaddr*)&client_addr, &addr_size);
    if (client_socket == -1)
    {
        std::cout << "accept failed" << std::endl;
        return;
    }
    if (fcntl(client_socket, F_SETFL, O_NONBLOCK) == -1)
    {
        std::cout << "blocking fd didnt succeed" << std::endl;
        close(client_socket);
        return;
    }
    Client client;
    client.setClientFd(client_socket);
    char clientIP[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, clientIP, INET_ADDRSTRLEN);
    client.setIP(std::string(clientIP));
    client_vect.push_back(client);
    
    struct pollfd client_pollfd;
    client_pollfd.fd = client_socket;
    client_pollfd.events = POLLIN;
    poll_fds.push_back(client_pollfd);
    
    std::cout << "New client connected: " << clientIP << " fd=" << client_socket << std::endl;
}

void Server::handleClientWrite()
{
    for (unsigned long i = 1; i < poll_fds.size(); i++)
    {
        if (poll_fds[i].revents & POLLOUT)
        {
            std::string& wbuf = client_vect[i-1].getWriteBuffer();  // ATTENZIONE: client_vect[i-1] corrisponde a poll_fds[i]
            if (!wbuf.empty())
            {
                ssize_t sent = send(poll_fds[i].fd, wbuf.c_str(), wbuf.size(), 0);
                if (sent > 0)
                {
                    wbuf.erase(0, sent);
                }
                else if (sent == -1 && errno != EAGAIN && errno != EWOULDBLOCK)
                {
                    // Errore grave: rimuovi client
                    close(poll_fds[i].fd);
                    poll_fds.erase(poll_fds.begin() + i);
                    client_vect.erase(client_vect.begin() + (i-1));
                    i--;  // perchè abbiamo rimosso l'elemento
                    continue;
                }
                if (wbuf.empty())
                {
                    poll_fds[i].events &= ~POLLOUT;
                }
            }
        }
    }
}

bool Server::handleClientRead(unsigned long i)
{
    char buffer[512];
    ssize_t bytes_letti;
    bytes_letti = recv(poll_fds[i].fd, buffer, sizeof(buffer) - 1, 0);
    if (bytes_letti > 0)
    {
        buffer[bytes_letti] = '\0';
        std::cout << "ricevuto da client " << i << ": " << buffer << std::endl;
        client_vect[i-1].getReadBuffer() += buffer;
        std::string& buf = client_vect[i-1].getReadBuffer();
        size_t pos;
        while ((pos = buf.find("\r\n")) != std::string::npos)
        {
            std::string command = buf.substr(0, pos);
            std::cout << "Comando completo: " << command << std::endl;
            buf.erase(0, pos + 2); // Qui puoi processare il comando
        }
    }
    else if (bytes_letti == 0) // client disconnesso
    {
        close(poll_fds[i].fd);
        poll_fds.erase(poll_fds.begin() + i);
        client_vect.erase(client_vect.begin() + (i-1));
        return true; // ho rimosso
    }
    else // bytes_letti == -1
    {
        if (errno != EAGAIN && errno != EWOULDBLOCK)
        {
            close(poll_fds[i].fd);
            poll_fds.erase(poll_fds.begin() + i);
            client_vect.erase(client_vect.begin() + (i-1));
            return true;
        }
    }
    return false;
}

void Server::sendToClient(int client_fd, const std::string& message)
{
    for (size_t i = 0; i < client_vect.size(); i++)
    {
        if (client_vect[i].getClientFd() == client_fd)
        {
            client_vect[i].getWriteBuffer() += message;
            poll_fds[i+1].events |= POLLOUT;  // +1 perchè poll_fds[0] è il server
            break;
        }
    }
}

void Server::run()
{
    std::cout << "Server running on port " << server_port << std::endl;
    for (;;)
    {
        if (poll(&poll_fds[0], poll_fds.size(), -1) == -1)
        {
            std::cout << "poll failed" << std::endl;
            return;
        }
        // Controlla se il server fd ha eventi
        if (poll_fds[0].revents & POLLIN)
            acceptNewClient();
        
        // Gestisci lettura da client
        for (unsigned long i = 1; i < poll_fds.size(); i++)
        {
            if (poll_fds[i].revents & POLLIN)
            {
                if (handleClientRead(i))
                {
                    // Se handleClientRead ha rimosso il client, l'indice i ora punta al prossimo
                    i--; // perchè il vettore si è rimpicciolito
                }
            }
        }
        // Gestisci scrittura verso client
        handleClientWrite();
    }
}

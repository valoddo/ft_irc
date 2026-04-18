/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vloddo <vloddo@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/13 14:04:12 by sel-khao          #+#    #+#             */
/*   Updated: 2026/04/18 17:38:55 by vloddo           ###   ########.fr       */
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
    return (socket_fd);
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

void Server::handleClientWrite() // Scrive dati al client (invia risposte)
{
    for (unsigned long i = 1; i < poll_fds.size(); i++)
    {
        if (poll_fds[i].revents & POLLOUT)
        {
            std::string& wbuf = client_vect[i].getWriteBuffer();  // Riportato come codice originale (ciclo con i=1 quindi poll_fds[i] gia mappato con client[i])
            if (!wbuf.empty())
            {
                ssize_t sent = send(poll_fds[i].fd, wbuf.c_str(), wbuf.size(), 0);
                if (sent > 0)
                {
                    wbuf.erase(0, sent);
                }
                else if (sent == -1)
                {
                    close(poll_fds[i].fd); // Errore grave: rimuovi client
                    poll_fds.erase(poll_fds.begin() + i);
                    client_vect.erase(client_vect.begin() + i);
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

bool Server::handleClientRead(unsigned long i) //Legge dati dal client (riceve messaggi)
{
    char buffer[512];
    ssize_t bytes_letti;
    bytes_letti = recv(poll_fds[i].fd, buffer, 512, 0);
    if (bytes_letti > 0)
    {
        buffer[bytes_letti] = '\0';
        std::cout << "ricevuto da client " << i << ": " << buffer << std::endl;
        client_vect[i-1].getReadBuffer() += buffer;
        std::string& buf = client_vect[i].getReadBuffer();
        size_t pos;
        while ((pos = buf.find("\r\n")) != std::string::npos)
        {
            std::string command = buf.substr(0, pos);
            std::cout << "Comando completo: " << command << std::endl;
            buf.erase(0, pos + 2); // Qui puoi processare il comando
            processCommand(client_vect[i], command);//INSERIRE FUNZIONE DI PROCESSCOMANDIRC
        }
    }
    else if (bytes_letti == 0) // client disconnesso
    {
        close(poll_fds[i].fd);
        poll_fds.erase(poll_fds.begin() + i);
        client_vect.erase(client_vect.begin() + i);
        i--;
        return true; // ho rimosso
    }
    else // bytes_letti == -1
    {
        if (errno != EAGAIN && errno != EWOULDBLOCK)
        {
            close(poll_fds[i].fd);
            poll_fds.erase(poll_fds.begin() + i);
            client_vect.erase(client_vect.begin() + i);
            return true;
        }
    }
    return false;
}

void Server::processCommand(Client& client, const std::string& command)
{
    size_t spacePos = command.find(' ');
    std::string cmd = command.substr(0, spacePos);
    for (size_t i = 0; i < cmd.size(); i++)
        cmd[i] = toupper(cmd[i]);
    
    std::string params = "";
    if (spacePos != std::string::npos)
        params = command.substr(spacePos + 1);
    
    if (cmd == "PASS")
    {
        execPass(client, params);
        return;
    }
    else if (cmd == "NICK")
    {
        execNick(client, params);
        return;
    }
    else if (cmd == "USER")
    {
        execUser(client, params);
        return;
    }
    if (!client.isAuthenticated())
    {
        client.getWriteBuffer() += "451 " + cmd + " :You have not registered\r\n";
        return;
    }
    if (cmd == "JOIN")
        execJoin(client, params);
    else if (cmd == "PRIVMSG")
        execPrivmsg(client, params);
    else if (cmd == "INVITE")
        execInvite(client, params);
    else if (cmd == "TOPIC")
        execTopic(client, params);
    else if (cmd == "MODE")
        execMode(client, params);
    else if (cmd == "KICK")
        execKick(client, params);
    else if (cmd == "QUIT")
        execQuit(client, params);
    else
    {
        client.getWriteBuffer() += "421 " + cmd + " :Unknown command\r\n";
    }
}

void Server::sendToClient(int client_fd, const std::string& message)
{
    for (size_t i = 0; i < client_vect.size(); i++)
    {
        if (client_vect[i].getClientFd() == client_fd)
        {
            client_vect[i].getWriteBuffer() += message;
            poll_fds[i].events |= POLLOUT; 
            break;
        }
    }
}

// DUBBIO: il ciclo dovrebbe partire da 1 perche 0 e un placeholder sia in poll_fds che client_vect, ma in caso ci 
//sia un problema nella cancellazione e piu sicuro fare il controllo di "i < client_vect.size() && i < poll_fds.size()"
// void Server::sendToClient(int client_fd, const std::string& message)
// {
//     for (size_t i = 1; i < client_vect.size() && i < poll_fds.size(); i++)
//     {
//         if (client_vect[i].getClientFd() == client_fd)
//         {
//             client_vect[i].getWriteBuffer() += message;
//             poll_fds[i].events |= POLLOUT; 
//             break;
//         }
//     }
// }

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
        if (poll_fds[0].revents & POLLIN) // Controlla se il server fd ha eventi
            acceptNewClient();
        for (unsigned long i = 1; i < poll_fds.size(); i++) // Gestisci lettura da client
        {
            if (poll_fds[i].revents & POLLIN)
            {
                if (handleClientRead(i))
                    i--; // Se handleClientRead ha rimosso il client, l'indice i ora punta al prossimo, perchè il vettore si è rimpicciolito
            }
        }
        handleClientWrite(); // Gestisci scrittura verso client
    }
}

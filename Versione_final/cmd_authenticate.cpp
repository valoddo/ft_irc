/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cmd_authenticate.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cacorrea <cacorrea@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/24 10:27:14 by cacorrea          #+#    #+#             */
/*   Updated: 2026/04/24 10:45:25 by cacorrea         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.hpp"

void Server::tryAuthenticate(Client& client)
{
    if (client.isAuthenticated()) return;
    if (client.getNick().empty() || client.getUser().empty()) return;
    if (!server_password.empty() && client.getPass() != server_password) return;
    client.setAuthenticated(true);
    std::string nick = client.getNick();
    client.getWriteBuffer() += ":" + getName() + " 001 " + nick + " :Welcome to " + getName() + "  " + client.getPrefix() + "\r\n";
    client.getWriteBuffer() += ":" + getName() + " 002 " + nick + " :Your host is " + getName() + "\r\n";
    client.getWriteBuffer() += ":" + getName() + " 003 " + nick + " :This server was created today\r\n";
    client.getWriteBuffer() += ":" + getName() + " 004 " + nick + " :" + getName() + " 1.0 o o\r\n";
    int fd = client.getClientFd();
    for (size_t i = 1; i < client_vect.size(); i++) {
        if (client_vect[i].getClientFd() == fd) {
            poll_fds[i].events |= POLLOUT;
            break;
        }
    }
}

//command use: PASS <password>
void Server::execPass(Client& client, const std::string& params)
{
    if (server_password.empty())
        return;
    if (!client.getPass().empty()){
        sendReply(client, ":" + getName() + " 462 " + client.getNick() + " :You may not reregister\r\n");
        return;
    }
    std::string pass = params;
    if (!pass.empty() && pass[0] == ':')
        pass = pass.substr(1);
    if (pass == server_password){
        client.setPass(pass);
        std::cout << "Client " << client.getClientFd() << " password accepted" << std::endl;
        tryAuthenticate(client);
    }
    else
        sendReply(client, ":" + getName() + " 464 " + client.getNick() + " :Password incorrect\r\n");
}

//command use: NICK <nickname>
void Server::execNick(Client& client, const std::string& params)
{
    if (params.empty()){
        sendReply(client, ":" + getName() + " 431 :No nickname given\r\n");
        return;
    }
    for (size_t i = 0; i < client_vect.size(); i++){
        if (client_vect[i].getClientFd() != client.getClientFd()){
            if (client_vect[i].getNick() == params){
                sendReply(client, ":" + getName() + " 433 " + params + " :Nickname already in use\r\n");
                return;
            }
        }
    }
    client.setNick(params);
	tryAuthenticate(client);
}

// command use: USER <username>
void Server::execUser(Client& client, const std::string& params) 
{
    if (params.empty()){
        sendReply(client, ":" + getName() + " 461 " + client.getNick() + " USER :Not enough parameters\r\n");
        return;
    }
	if (!client.getUser().empty()){
        sendReply(client, ":" + getName() + " 462 " + client.getNick() + " :You may not reregister\r\n");
        return;
    }
	size_t spacePos = params.find(' ');
    std::string username = params;
    if (spacePos != std::string::npos)
        username = params.substr(0, spacePos);
    client.setUser(username);
	tryAuthenticate(client);
}

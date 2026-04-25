/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   command.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cacorrea <cacorrea@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/19 19:38:50 by sel-khao          #+#    #+#             */
/*   Updated: 2026/04/25 18:15:08 by cacorrea         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.hpp"

// command use: JOIN <channel> [<key>]
void Server::execJoin(Client& client, const std::string& params) 
{
    if (params.empty())
    {
        sendReply(client, ":" + getName() + " 461 " + client.getNick() + " JOIN :Not enough parameters\r\n");
        return;
    }
    size_t spacePos = params.find(' ');
    std::string channelName;
    std::string password;
    if (spacePos == std::string::npos)
    {
        channelName = params;
        password = "";
    }
    else
    {
        channelName = params.substr(0, spacePos);
        password = params.substr(spacePos + 1);
    }
    if (channelName.empty())
    {
        sendReply(client, ":" + getName() + " 461 " + client.getNick() + " JOIN :Not enough parameters\r\n");
        return;
    }
    if(channelName[0] != '#')
	{
        sendReply(client, ":" + getName() + " 403 " + client.getNick() + " " + channelName + " :No such channel\r\n");
        return;
    }
    std::map<std::string, Channel>::iterator it = channels.find(channelName);
    if (it == channels.end())
    {
        channels[channelName] = Channel(channelName);
        it = channels.find(channelName);
    }
    it->second.processJoin(client, client_vect, password);
}

//commmand use: INVITE <nickname> <#canale>
void Server::execInvite(Client& client, const std::string& params)
{
    if (params.empty())
    {
        sendReply(client, ":" + getName() + " 461 " + client.getNick() + " INVITE :Not enough parameters\r\n");
        return;
    }
    size_t spacePos = params.find(' ');
    std::string targetnick = params.substr(0, spacePos);
    std::string channel = params.substr(spacePos + 1);

    std::map<std::string, Channel>::iterator it = channels.find(channel);
    if (it == channels.end())
    {
        sendReply(client, ":" + getName() + " 403 " + client.getNick() + " " + channel + " :No such channel\r\n");
        return;
    }
    bool found = false;
    std::vector<Client>::iterator it_client = client_vect.begin();
    for (size_t i = 0; i < client_vect.size(); i++)
    {
        if (client_vect[i].getNick() == targetnick)
        {
            found = true;
            break ;
        }
        it_client++;
    }
	if (!found){
        sendReply(client,  ":" + getName() + " 401 " + client.getNick() + " " + targetnick + " :No such nick\r\n");
        return;
    }
    it->second.processInvite(client, *it_client);
}

//command use: QUIT [<Quit message>]
void Server::execQuit(Client& client, const std::string& params){
	std::string quitMessage = "Leaving";
	if (!params.empty() && params[0] == ':')
		quitMessage = params.substr(1);
	std::string quitMsg = ":" + client.getPrefix() + " QUIT :" + quitMessage;

    for (std::map<std::string, Channel>::iterator it = channels.begin(); it != channels.end(); ++it){
		if (it->second.isMember(client)){
			it->second.broadcast(quitMsg, client_vect);
			it->second.removeClient(client);
		}
	}
	close(client.getClientFd());
	for (size_t i = 1; i < poll_fds.size() && i < client_vect.size(); i++){
		if (client_vect[i].getClientFd() == client.getClientFd()){
			poll_fds.erase(poll_fds.begin() + i);
			client_vect.erase(client_vect.begin() + i);
            std::cout << "Client " << client.getClientFd() << " disconnected" << std::endl;
			break;
		}
	}
}

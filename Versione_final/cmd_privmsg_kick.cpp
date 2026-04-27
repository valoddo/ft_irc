/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cmd_privmsg_kick.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vloddo <vloddo@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/24 11:50:22 by cacorrea          #+#    #+#             */
/*   Updated: 2026/04/27 16:47:32 by vloddo           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.hpp"

//command use: PRIVMSG <receiver> <text to be sent>
void Server::execPrivmsg(Client& client, const std::string& params)
{
    if (params.empty()) {
        sendReply(client, ":" SERVER_NAME " 411 " + client.getNick() + " :No recipient given\r\n");
        return;
    }
    size_t spacePos = params.find(' '); 
    if (spacePos == std::string::npos) {
        sendReply(client, ":" SERVER_NAME " 411 " + client.getNick() + " :No recipient given\r\n");
        return;
    }
    std::string target = params.substr(0, spacePos);
    if (target.empty()) {
        sendReply(client, ":" SERVER_NAME " 411 " + client.getNick() + " :No recipient given\r\n");
        return;
    }
    std::string message = params.substr(spacePos + 1);
    if (message.empty()) {
        sendReply(client, ":" SERVER_NAME " 412 " + client.getNick() + " :No text to send\r\n");
        return;
    }
    if (message[0] == ':')
        message = message.substr(1);
    else {
        sendReply(client, ":" SERVER_NAME " 412 " + client.getNick() + " :No text to send\r\n");
        return;
    }
    std::string fullMsg = ":" + client.getPrefix() + " PRIVMSG " + target + " :" + message;
    if (target[0] == '#') { // Caso canale
        std::map<std::string, Channel>::iterator it = channels.find(target);
        if (it == channels.end()) {
            sendReply(client, ":" SERVER_NAME " 401 " + client.getNick() + " " + target + " :No such nick/channel\r\n");
            return;
        }
        if (!it->second.isMember(client)) {
            sendReply(client, ":" SERVER_NAME " 404 " + client.getNick() + " " + target + " :Cannot send to channel\r\n");
            return;
        }
        it->second.broadcast(fullMsg, client_vect, client.getClientFd());
        return;
    }
    for (size_t i = 0; i < client_vect.size(); ++i) {
        if (client_vect[i].getNick() == target) {
            client_vect[i].getWriteBuffer() += fullMsg + "\r\n";
            poll_fds[i].events |= POLLOUT;
            return;
        }
    }
    sendReply(client, ":" SERVER_NAME " 401 " + client.getNick() + " " + target + " :No such nick/channel\r\n");
}

void Server::execKick(Client& client, const std::string& params){
	if (params.empty()){
		sendReply(client, ":" SERVER_NAME " 461 " + client.getNick() + " KICK :Not enough parameters\r\n");
        return ;
	}
	size_t spacePos = params.find(' ');
    if (spacePos == std::string::npos){
        sendReply(client, ":" SERVER_NAME " 461 " + client.getNick() + " KICK :Not enough parameters\r\n");
        return ;
    }
	std::string channelName = params.substr(0, spacePos);
    std::string rest = params.substr(spacePos + 1);
	size_t spacePos2 = rest.find(' ');
    std::string targetNick;
    std::string reason;
	if (spacePos2 == std::string::npos){
		targetNick = rest;
		reason = client.getNick();
	}
	else{
		targetNick = rest.substr(0, spacePos2);
		std::string reasonPart = rest.substr(spacePos2 + 1);
		if (!reasonPart.empty() && reasonPart[0] == ':')
			reason = reasonPart.substr(1);
		else
			reason = client.getNick();
	}
    std::map<std::string, Channel>::iterator it = channels.find(channelName);
    if (it == channels.end()){
        sendReply(client, ":" SERVER_NAME " 403 " + client.getNick() + " " + channelName + " :No such channel\r\n");
        return ;
    }
    Channel& channel = it->second;
	if (!channel.isMember(client)){
        sendReply(client, ":" SERVER_NAME " 442 " + client.getNick() + " " + channelName + " :You're not on that channel\r\n");
        return ;
	}
	if (!channel.isOperator(client)){
        sendReply(client, ":" SERVER_NAME " 482 " + client.getNick() + " " + channelName + " :You're not a channel operator\r\n");
        return ;
	}
	Client* targetClient = NULL;
	for (size_t i = 1; i < client_vect.size(); i++){
		if (client_vect[i].getNick() == targetNick){
			targetClient = &client_vect[i];
			break ;
		}
	}
	if (!targetClient){
		sendReply(client, ":" SERVER_NAME " 401 " + client.getNick() + " " + targetNick + " :No such nick\r\n");
        return ;
	}
	if (!channel.isMember(*targetClient)){
        sendReply(client, ":" SERVER_NAME " 441 " + client.getNick() + " " + targetNick + " " + channelName + " :They aren't on that channel\r\n");
        return ;
	}
	std::string kickMsg = ":" + client.getPrefix() + " KICK " + channelName + " " + targetNick + " :" + reason;
	channel.broadcast(kickMsg, client_vect);
	channel.removeClient(*targetClient);
}

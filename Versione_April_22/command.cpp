/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   command.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sel-khao <sel-khao@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/19 19:38:50 by sel-khao          #+#    #+#             */
/*   Updated: 2026/04/22 15:38:21 by sel-khao         ###   ########.fr       */
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
    client.getWriteBuffer() += ":" + getName() + " 004 " + nick + getName() + " 1.0 o o\r\n";
    int fd = client.getClientFd();
    for (size_t i = 1; i < client_vect.size(); i++) {
        if (client_vect[i].getClientFd() == fd) {
            poll_fds[i].events |= POLLOUT;
            break;
        }
    }
}

void Server::execPass(Client& client, const std::string& params)
{
    if (server_password.empty())
        return;
    if (!client.getPass().empty()){
        sendReply(client, "462 :You may not reregister\r\n");
        return;
    }
    std::string pass = params;
    if (!pass.empty() && pass[0] == ':')
        pass = pass.substr(1);
    if (pass == server_password){
        client.setPass(params);
        std::cout << "Client " << client.getClientFd() << " password accepted" << std::endl;
        tryAuthenticate(client);
    }
    else
        sendReply(client, "464 :Password incorrect\r\n");
}

void Server::execNick(Client& client, const std::string& params) // NICK <nickname>
{
    if (params.empty()){
        sendReply(client, "431 :No nickname given\r\n");
        return;
    }
    for (size_t i = 0; i < client_vect.size(); i++){
        if (client_vect[i].getClientFd() != client.getClientFd()){
            if (client_vect[i].getNick() == params){
                sendReply(client, "433 :Nickname already in use\r\n");
                return;
            }
        }
    }
    client.setNick(params);
	tryAuthenticate(client);
}

void Server::execUser(Client& client, const std::string& params) // USER <username> <hostname> <servername> :<realname>
{
    if (params.empty()){
        sendReply(client, "461 USER :Not enough parameters\r\n");
        return;
    }
	if (!client.getUser().empty()){
        sendReply(client, "462 :You may not reregister\r\n");
        return;
    }
	size_t spacePos = params.find(' '); // Parsa solo il primo parametro (username)
    std::string username = params;
    if (spacePos != std::string::npos)
        username = params.substr(0, spacePos);
    client.setUser(username);
	tryAuthenticate(client);
}

void Server::execJoin(Client& client, const std::string& params) // JOIN <#canale>
{
    if (params.empty())
    {
        sendReply(client, "461 JOIN :Not enough parameters\r\n");
        return;
    }
    size_t spacePos = params.find(' ');
    std::string channelName;
    std::string password;
    if (spacePos == std::string::npos)
    {
        channelName = params; // Nessuno spazio trovato → solo il nome del canale, senza password
        password = "";
    }
    else
    {
        channelName = params.substr(0, spacePos); // C'è uno spazio → prendi canale prima e password dopo
        password = params.substr(spacePos + 1);
    }
    if (channelName.empty()) // Verifica che il channelName non sia vuoto
    {
        sendReply(client, "461 JOIN :Not enough parameters\r\n");
        return;
    }
    if(channelName[0] != '#')
	{
        sendReply(client, "403 " + channelName + " :No such channel\r\n");
        return;
    }
    std::map<std::string, Channel>::iterator it = channels.find(channelName); // Crea il canale se non esiste
    if (it == channels.end())
    {
        channels[channelName] = Channel(channelName); // Canale non esiste, lo creiamo
        it = channels.find(channelName);
    }
    std::string server_name = getName();
    it->second.processJoin(client, password, server_name); // Ora processa il JOIN sul canale
}

void Server::execInvite(Client& client, const std::string& params) // INVITE <nickname> <#canale>
{
    if (params.empty())
    {
        sendReply(client, "461 INVITE :Not enough parameters\r\n");
        return;
    }
    size_t spacePos = params.find(' '); // Parsa solo il primo parametro (destinatario)
    std::string targetnick = params.substr(0, spacePos);
    std::string channel = params.substr(spacePos + 1);

    std::map<std::string, Channel>::iterator it = channels.find(channel);
    if (it == channels.end())
    {
        sendReply(client, "403 " + channel + " :No such channel\r\n");
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
        sendReply(client,  "401 " + targetnick + " :No such nick\r\n");
        return;
    }
    it->second.processInvite(client, *it_client); // DELEGA al canale
}



static  std::vector<std::string> extractTokens(const std::string& input)
{
	std::vector<std::string> 	tokens;
	std::string					token;
	std::stringstream			ss(input);

	while (ss >> token)
	{
		tokens.push_back(token);
	}
	return (tokens);
}

static  std::vector<std::string>    littelParsing(std::string& parameters)
{
	size_t		pos;
	std::string	msg;
	bool		hasMsg = false;
    std::vector<std::string>    result;
	
	pos = parameters.find(":");
	if (pos != std::string::npos)
	{//divide in due eliminando i :
		msg = parameters.substr(pos + 1);
		parameters = parameters.substr(0, pos);
		hasMsg = true;
	}
	result = extractTokens(parameters);
	if (hasMsg)
		result.push_back(msg);
	return (result);
}


void    Server::execTopic(Client &client, std::string &params) // TOPIC <#canale> :<nuovo topic>
{
    std::vector<std::string>    parameters;
    //size_t                      i = 0;
    //bool                        goodName = false;

    parameters = littelParsing(params);
    if (parameters.empty())//461 ERR_NEEDMOREPARAMS
    {
        sendReply(client, ":" + getName() + " 461 " + client.getNick() + " TOPIC :Not enough parameters\r\n");
        return;
    }
    std::string channelName = parameters[0];
    //se non esiste il canale 403 ERR_NOSUCHCHANNEL
    if (channelName.empty() || channelName[0] != '#')
    {
        sendReply(client, ":" + getName() + " 403 " + client.getNick() + " " + channelName + " :No such channel\r\n");
        return;
    }
    std::map<std::string, Channel>::iterator it = channels.find(channelName);
	if (it == channels.end())
    {
        sendReply(client,":" + getName() + " 403 " + client.getNick() + " " + channelName + " :No such channel\r\n");
        return;
    }
	Channel	&ch = it->second;
    //se c'e solo il nome del canale stampa il topic
    if (parameters.size() == 1)
    {
		if (ch.getTopic().empty())// RPL_NOTOPIC (331)
            sendReply(client,":" + getName() + " 331 " + client.getNick() + " " + channelName + " :No topic is set\r\n");
        else// RPL_TOPIC (332)
            sendReply(client,":" + getName() + " 332 " + client.getNick() + " " + channelName + " :" + ch.getTopic() + "\r\n");
    }
    //piu di 1 parametro: Change/set the Topic (TOPIC #channel :new topic)
    else 
    {
		if (!ch.isMember(client))//442 ERR_NOTONCHANNEL
		{
			sendReply(client,":" + getName() + " 442 " + client.getNick() + " " + channelName + " :You're not on that channel\r\n");
			return;
		}
        if (ch.getTopicRestrict() && !(ch.isOperator(client)))//482 ERR_CHANOPRPRIVSNEEDED
		{
			sendReply(client,":" + getName() + " 482 " + client.getNick() + " " + channelName + " :You're not a channel operator\r\n");
			return;
		}
        // change the topic and broadcast the change to everyone in the channel
        ch.setTopic(parameters[1]);
        //>> :cacorreaa!~carolina@185.30.66.235 TOPIC #my42irc :papere
		std::string	msg = ":" + client.getPrefix() + " TOPIC " + channelName + " :" + ch.getTopic();
		ch.broadcast(msg, NULL);
    }
}

/*
PRIVMSG <destinatario> :<messaggio>
se destinatario inizia con # allora e' un channel: so send to all memebers of the channel
otherwise its an user, so send only privately to him c:

PRIVMSG #canale :Ciao a tutti
PRIVMSG marco :Ciao come stai?

msg must be formatted
":nick!user@host PRIVMSG destinatario :messaggio"
":marco!marco@192.168.1.5 PRIVMSG #canale :Ciao a tutti"

*/

void Server::execPrivmsg(Client& client, const std::string& params)
{
    if (params.empty()) {
        sendReply(client, "411 :No recipient given\r\n");
        return;
    }
    size_t spacePos = params.find(' '); 
    if (spacePos == std::string::npos) {
        sendReply(client, ":" + getName() + " 411 " + client.getNick() + " :No recipient given\r\n");
        return;
    }
    std::string target = params.substr(0, spacePos);
    if (target.empty()) {
        sendReply(client, ":" + getName() + " 411 " + client.getNick() + " :No recipient given\r\n");
        return;
    }
    std::string message = params.substr(spacePos + 1);
    if (message.empty()) {
        sendReply(client, ":" + getName() + " 412 " + client.getNick() + " :No text to send\r\n");
        return;
    }
    if (message[0] == ':')
        message = message.substr(1);
    else {
        sendReply(client, "412 :No text to send\r\n");
        return;
    }
    std::string fullMsg = ":" + client.getPrefix() + " PRIVMSG " + target + " :" + message;
    if (target[0] == '#') { // Caso canale
        std::map<std::string, Channel>::iterator it = channels.find(target);
        if (it == channels.end()) {
            sendReply(client, ":" + getName() + " 401 " + client.getNick() + " " + target + " :No such nick/channel\r\n");
            return;
        }
        if (!it->second.isMember(client)) { // Verifica che il client sia membro del canale
            sendReply(client, ":" + getName() + " 404 " + client.getNick() + " " + target + " :Cannot send to channel\r\n");
            return;
        }
        it->second.broadcast(fullMsg, &client); // Invia a tutti i membri tranne il mittente
        return;
    }
    for (size_t i = 0; i < client_vect.size(); ++i) { // Caso nickname
        if (client_vect[i].getNick() == target) {
            client_vect[i].getWriteBuffer() += fullMsg;
            poll_fds[i].events |= POLLOUT;
            return;
        }
    }
    sendReply(client, ":" + getName() + " 401 " + client.getNick() + " " + target + " :No such nick/channel\r\n"); // Nessun destinatario trovato
}


//manca il localhost all'inizio dei messaggi (server name)
void Server::execMode(Client& client, std::string& params)// MODE <channel> [modi] [parametri]
{
    std::vector<std::string>    parameters;

    parameters = littelParsing(params);
    if (params.empty())//461 ERR_NEEDMOREPARAMS
    {
        sendReply(client,":" + getName() + " 461 " + client.getNick() + " MODE :Not enough parameters\r\n");
        return;
    }
    if (parameters.size() >= 1)
    {
        std::string channelName = parameters[0];
        //se non esiste il canale
        if (channelName.empty() || channelName[0] != '#')
        {
            sendReply(client,":" + getName() + " 403 " + client.getNick() + " " + channelName + " :No such channel\r\n");
            return;
        }
        std::map<std::string, Channel>::iterator it = channels.find(channelName);
        if (it == channels.end())
        {
            sendReply(client,":" + getName() + " 403 " + client.getNick() + " " + channelName + " :No such channel\r\n");
            return;
        }
        Channel& ch = it->second;
        ch.processMode(client, parameters);
    }
} 



/* The commands which may only be used by channel operators are:
        KICK    - Eject a client from the channel
        MODE    - Change the channel's mode
        INVITE  - Invite a client to an invite-only channel (mode +i)
        TOPIC   - Change the channel topic in a mode +t channel
i: Set/remove Invite-only channel                                               1 param
t: Set/remove the restrictions of the TOPIC command to channel operators        1 param
k: Set/remove the channel key (password)                                        2 param
o: Give/take channel operator privilege                                         2 param
l: Set/remove the user limit to channel                                         2 param

*/


/*

when operatore fa "KICK #canale nickname :motivo"
1. verifica esiste canale
2. verificaa che chi fa kick sia l'operatore
3. verifica che il target esiste e e' nel canale
4. remove him
5. boardcast it ":kicker!user@host KICK #canale target :motivo"

formato comando:
KICK #canale marco :Troppo spam

fomato boardcast: 
:marco!marco@192.168.1.5 KICK #canale luca :Troppo spam
*/

void Server::execKick(Client& client, const std::string& params){
	if (params.empty()){
		sendReply(client, "461 KICK :Not enough parameters\r\n");
        return ;
	}
	//separo primo spazio separa canale dal resto
	size_t spacePos = params.find(' ');
    if (spacePos == std::string::npos){
        sendReply(client, "461 KICK :Not enough parameters\r\n");
        return ;
    }
	std::string channelName = params.substr(0, spacePos);
    std::string rest = params.substr(spacePos + 1);
	//separato nick da motivo
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
        sendReply(client, "403 " + channelName + " :No such channel\r\n");
        return ;
    }
    Channel& channel = it->second;
	if (!channel.isMember(client)){
        sendReply(client, "442 " + channelName + " :You're not on that channel\r\n");
        return ;
	}
	if (!channel.isOperator(client)){
        sendReply(client, "482 " + channelName + " :You're not a channel operator\r\n");
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
		sendReply(client, "401 " + targetNick + " :No such nick\r\n");
        return ;
	}
	if (!channel.isMember(*targetClient)){
        sendReply(client, "441 " + targetNick + " " + channelName + " :They aren't on that channel\r\n");
        return ;
	}
	std::string kickMsg = ":" + client.getPrefix() + " KICK " + channelName + " " + targetNick + " :" + reason + "\r\n";
	channel.broadcast(kickMsg, NULL);//broadcasto a tutti nel canale
	channel.removeClient(*targetClient);//imuovo il target dal canale
}

/*
il cliente se ne va via da se'. tutti must know who went away.
server deve liberare tutte le risorse.
    parse
e per ogni channel il client was in:
    :<nickname>!<username>@<ip> QUIT :<messaggio>
    remove client from list
close socket of client
remove client from poll_fds e client_vect

*/
void Server::execQuit(Client& client, const std::string& params){//params is mess opzionale dopo QUIT
	std::string quitMessage = "Quit";//se client non da un messaggio then protocollo irc chiede defult quit
	if (!params.empty() && params[0] == ':')
		quitMessage = params.substr(1);
	std::string quitMsg = ":" + client.getPrefix() + " QUIT :" + quitMessage;//formato irc x annunciare quit "":nickname!username@ip QUIT :messaggio"
	//ora brodcast a tutti i channel
	for (std::map<std::string, Channel>::iterator it = channels.begin(); it != channels.end(); ++it){
		if (it->second.isMember(client)){
			it->second.broadcast(quitMsg, NULL);//NULL cuz nessuno must be escluso
			it->second.removeClient(client);
		}
	}
	close(client.getClientFd());//free resources in the os
	for (size_t i = 1; i < poll_fds.size() && i < client_vect.size(); i++){
		if (client_vect[i].getClientFd() == client.getClientFd()){
			poll_fds.erase(poll_fds.begin() + i);
			client_vect.erase(client_vect.begin() + i);
			break;
		}
	}
}
//when client esce must remove from both lists, same position or sono desincronizzati i = 1 cuz 0 is socket server

//getPrefix() restituisce nickname!username@ip
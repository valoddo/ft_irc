#include "server.hpp"

void Server::tryAuthenticate(Client& client)
{
    if (client.isAuthenticated())
        return;
    if (client.getNick().empty() || client.getUser().empty())
        return;
    if (!server_password.empty() && client.getPass() != server_password)
        return;
    client.setAuthenticated(true);
    client.getWriteBuffer() += "001 " + client.getNick() + " :Welcome\r\n";
	std::cout << "Client " << client.getNick() << " authenticated!" << std::endl;
}

void Server::execPass(Client& client, const std::string& params) // PASS <password>
{
    // Se server non richiede password, ignora
    if (server_password.empty())
        return;
    if (!client.getPass().empty())
    {
        client.getWriteBuffer() += "462 :You may not reregister\r\n";
        return;
    }
    if (params == server_password)
    {
        client.setPass(params);
        std::cout << "Client " << client.getClientFd() << " password accepted" << std::endl;
		tryAuthenticate(client);
	}
    else
    {
        client.getWriteBuffer() += "464 :Password incorrect\r\n";
    }
}

void Server::execNick(Client& client, const std::string& params) // NICK <nickname>
{
    if (params.empty())
    {
        client.getWriteBuffer() += "431 :No nickname given\r\n";
        return;
    }
    for (size_t i = 0; i < client_vect.size(); i++)
    {
        if (client_vect[i].getClientFd() != client.getClientFd())
        {
            if (client_vect[i].getNick() == params)
            {
                client.getWriteBuffer() += "433 " + params + " :Nickname already in use\r\n";
                return;
            }
        }
    }
    client.setNick(params);
	tryAuthenticate(client);
}

void Server::execUser(Client& client, const std::string& params) // USER <username> <hostname> <servername> :<realname>
{
    if (params.empty())
    {
        client.getWriteBuffer() += "461 USER :Not enough parameters\r\n";
        return;
    }
	if (!client.getUser().empty())
    {
        client.getWriteBuffer() += "462 :You may not reregister\r\n";
        return;
    }
	size_t spacePos = params.find(' '); // Parsa solo il primo parametro (username)
    std::string username = params;
    if (spacePos != std::string::npos)
        username = params.substr(0, spacePos);
    client.setUser(params);
	tryAuthenticate(client);
}

void Server::execJoin(Client& client, const std::string& params) // JOIN <#canale>
{
    if (params.empty())
    {
        client.getWriteBuffer() += "461 JOIN :Not enough parameters\r\n";
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
        client.getWriteBuffer() += "461 JOIN :Not enough parameters\r\n";
        return;
    }
    std::map<std::string, Channel>::iterator it = channels.find(channelName); // Crea il canale se non esiste
    if (it == channels.end())
    {
        channels[channelName] = Channel(channelName); // Canale non esiste, lo creiamo
        it = channels.find(channelName);
    }
    it->second.processJoin(client, password); // Ora processa il JOIN sul canale
}



void Server::execInvite(Client& client, const std::string& params) // INVITE <nickname> <#canale>
{
    if (params.empty())
    {
        client.getWriteBuffer() += "461 INVITE :Not enough parameters\r\n";
        return;
    }
    size_t spacePos = params.find(' '); // Parsa solo il primo parametro (destinatario)
    std::string targetnick = params.substr(0, spacePos);
    std::string channel = params.substr(spacePos + 1);

    std::map<std::string, Channel>::iterator it = channels.find(channel);
    if (it == channels.end())
    {
        client.getWriteBuffer() += "403 " + channel + " :No such channel\r\n";
        return;
    }
    bool found = false;
    for (size_t i = 0; i < client_vect.size(); i++)
    {
        if (client_vect[i].getNick() == targetnick)
        {
            found = true;
            break;
        }
    if(found == false)
        {
            client.getWriteBuffer() += "401 " + targetnick + " :No such nick\r\n";
            return;
        }
    }
    it->second.processInvite(client, targetnick); // DELEGA al canale
}


void Server::execTopic(Client& client, const std::vector<std::string>& params)// TOPIC <#canale> :<nuovo topic>
{
    size_t  i = 0;
    bool    goodName = false;

    if (params.empty())
    {
        client.getWriteBuffer() += "461 " + client.getNick() + " TOPIC :Not enough parameters\r\n";
        return;
    }
    std::string channelName = params[0];
    //se non esiste il canale
    if (channelName.empty() || channelName[0] != '#')
    {
        client.getWriteBuffer() += "403 " + client.getNick() + " " + channelName + 
		" :No such channel\r\n";
        return;
    }
    std::map<std::string, Channel>::iterator it = channels.find(channelName);
	if (it == channels.end())
    {
        client.getWriteBuffer() += "403 " + client.getNick() + " " + channelName + 
		" :No such channel\r\n";
        return;
    }
	Channel	&ch = it->second;
    //se c'e solo il nome del canale stampa il topic
    if (params.size() == 1)
    {
		if (ch.getTopic().empty())// RPL_NOTOPIC (331)
        {
            client.getWriteBuffer() += "331 " + client.getNick() + " " + channelName + 
			" :No topic is set\r\n";
        }
        else// RPL_TOPIC (332)
        {
            client.getWriteBuffer() += "332 " + client.getNick() + " " + channelName + 
			" :" + ch.getTopic() + "\r\n";
		}
    }
    //piu di 1 parametro: Change/set the Topic (TOPIC #channel :new topic)
    else 
    {
		if (!ch.isMember(&client))// ERR_NOTONCHANNEL
		{
			client.getWriteBuffer() += "442 " + client.getNick() + " " + channelName + 
			" :You're not on that channel\r\n";
			return;
		}
        if (ch.getTopicRestrict() && !(ch.isOperator(&client)))//ERR_CHANOPRPRIVSNEEDED
		{
			client.getWriteBuffer() += "482 " + client.getNick() + " " + channelName + 
			" :You're not a channel operator\r\n";
			return;
		}
        // change the topic and broadcast the change to everyone in the channel
        ch.setTopic(params[1]);
		std::string	msg = ":" + client.getPrefix() + " TOPIC " + channelName + " :" + params[1];
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

static std::string parsInizPrivmsg(Client& client, const std::string& params, std::string& fullMsg){
	if (params.empty()){//if no command
		client.getWriteBuffer() += "411 :No recipient given\r\n";
		return "";
	}
	size_t spacePos = params.find(' ');//first space to separare msg e destinatario
	if (spacePos == std::string::npos){
		client.getWriteBuffer() += "412 :No text to send\r\n";
		return "";
	}
	std::string target = params.substr(0, spacePos);//#canale or marco
	std::string message = params.substr(spacePos + 1);//msg
	if (!message.empty() && message[0] == ':')
		message = message.substr(1);//message must strat with :
	else {
		client.getWriteBuffer() += "412 :No text to send\r\n";
		return "";
	}//build msg to send
	fullMsg = ":" + client.getPrefix() + " PRIVMSG " + target + " :" + message + "\r\n";
	return target;
}


void Server::execPrivmsg(Client& client, const std::string& params){
	std::string fullMsg;
	std::string target = parsInizPrivmsg(client, params, fullMsg);
	if (target[0] == '#'){
		std::map<std::string, Channel>::iterator it = channels.find(target);//search for channel
		if (it == channels.end()) {
			client.getWriteBuffer() += "401 " + target + " :No such nick/channel\r\n";
			return ;
		}
		//check if client is a member of channel
		if (!it->second.isMember(client)){
			client.getWriteBuffer() += "404 " + target + " :Cannot send to channel\r\n";
			return ;
		}
		it->second.broadcast(fullMsg, &client);
	}
	else{
		for (size_t i = 1; i < client_vect.size(); i++){
			if (client_vect[i].getNick() == target){
				client_vect[i].getWriteBuffer() += fullMsg;
				poll_fds[i].events |= POLLOUT;
				return ;
			}
		}
		//in caso couldnt find it
		client.getWriteBuffer() += "401 " + target + " :No such nick/channel\r\n";
	}
}

//void execTopic(Client& client, const std::string& params); // TOPIC <#canale> :<nuovo topic>

void Server::execMode(Client& client, const std::string& params){

} // MODE <target> <modi> [parametri]

void Server::execKick(Client& client, const std::string& params){

} // KICK <#canale> <nickname> [motivo]


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
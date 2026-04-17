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



void execPrivmsg(Client& client, const std::string& params); // PRIVMSG <destinatario> :<messaggio>





void Server::execInvite(Client& client, const std::string& params) // INVITE <nickname> <#canale>
{
    if (params.empty())
    {
        client.getWriteBuffer() += "461 INVITE :Not enough parameters\r\n";
        return;
    }
    size_t spacePos = params.find(' '); // Parsa solo il primo parametro (destinatario)
    std::string targetnick = params.substr(0, spacePos);
    std::string messagge = params.substr(spacePos + 1);

    std::map<std::string, Channel>::iterator it = channels.find(targetnick);
    if (it == channels.end())
    {
        client.getWriteBuffer() += "403 " + targetnick + " :No such channel\r\n";
        return;
    }
    Channel& channel = it->second;
    Client* target = NULL;
    for (size_t i = 0; i < client_vect.size(); i++)
    {
        if (client_vect[i].getNick() == targetnick)
        {
            target = &client_vect[i];
            break;
        }
    }
    if (!target)
    {
        client.getWriteBuffer() += "401 " + targetnick + " :No such nick\r\n";
        return;
    }
    channel.processInvite(client, *target); // DELEGA al canale
}






void execTopic(Client& client, const std::string& params); // TOPIC <#canale> :<nuovo topic>
void execMode(Client& client, const std::string& params); // MODE <target> <modi> [parametri]
void execKick(Client& client, const std::string& params); // KICK <#canale> <nickname> [motivo]
void execQuit(Client& client, const std::string& params); // QUIT [messaggio]



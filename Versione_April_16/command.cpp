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




void Server::execPass(Client& client, const std::string& params)
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
void Server::execNick(Client& client, const std::string& params)
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
void Server::execUser(Client& client, const std::string& params)
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

void execJoin(Client& client, const std::string& params);
void execPrivmsg(Client& client, const std::string& params);
void execQuit(Client& client, const std::string& params);
void execInvite(Client& client, const std::string& params);
void execTopic(Client& client, const std::string& params);
void execMode(Client& client, const std::string& params);
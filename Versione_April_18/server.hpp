/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vloddo <vloddo@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/13 14:04:19 by sel-khao          #+#    #+#             */
/*   Updated: 2026/04/18 20:25:11 by vloddo           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include "client.hpp"
#include "channel.hpp"

#include <iostream>
#include <cstdlib>
#include <poll.h>
#include <sys/time.h>
#include <vector>
#include <cerrno>
#include <sys/unistd.h>
#include <sys/fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h> 
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <map>

class Server
{
	private:
		std::vector<struct pollfd>	poll_fds;
		std::vector<Client>			client_vect;
		std::map<std::string, Channel> channels; // mappa il nome del canale all'oggetto Channel
		int							socket_fd;
		std::string					server_port;
		std::string					server_password;

	public:
		Server(const std::string& port, const std::string& password);
		~Server();
		void run();
		void initServer();                // chiama setupServer
		int setupServer(int port);        // privato
		
		void acceptNewClient();           // usa i membri
		bool handleClientRead(size_t i);  // i = indice in poll_fds
		void handleClientWrite();         // itera su poll_fds
		void sendToClient(int client_fd, const std::string& message);
		void processCommand(Client& client, const std::string& command);
		void tryAuthenticate(Client& client);
		
		void execPass(Client& client, const std::string& params); // PASS <password>
		void execNick(Client& client, const std::string& params); // NICK <nickname>
		void execUser(Client& client, const std::string& params); // USER <username> <hostname> <servername> :<realname>
		void execJoin(Client& client, const std::string& params); // JOIN <#canale>
		void execPrivmsg(Client& client, const std::string& params); // PRIVMSG <destinatario> :<messaggio>
		void execInvite(Client& client, const std::string& params); // INVITE <nickname> <#canale>
		void execTopic(Client& client, const std::string& params); // TOPIC <#canale> :<nuovo topic>
		void execMode(Client& client, const std::string& params); // MODE <target> <modi> [parametri]
		void execKick(Client& client, const std::string& params); // KICK <#canale> <nickname> [motivo]
		void execQuit(Client& client, const std::string& params); // QUIT [messaggio]
};



#endif
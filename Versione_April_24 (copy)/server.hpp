/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cacorrea <cacorrea@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/13 14:04:19 by sel-khao          #+#    #+#             */
/*   Updated: 2026/04/24 12:07:28 by cacorrea         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

# include "client.hpp"
# include "channel.hpp"

# include <iostream>
# include <cstdlib>
# include <poll.h>
# include <sys/time.h>
# include <vector>
# include <cerrno>
# include <sys/unistd.h>
# include <sys/fcntl.h>
# include <netdb.h>
# include <netinet/in.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <arpa/inet.h> 
# include <string>
# include <unistd.h>
# include <fcntl.h>
# include <map>

class Server
{
	private:
		std::string 					server_name;
		std::vector<struct pollfd>		poll_fds;
		std::vector<Client>				client_vect;
		std::map<std::string, Channel>	channels;
		int								socket_fd;
		std::string						server_port;
		std::string						server_password;

	public:
		Server(const std::string& port, const std::string& password);
		~Server();
		void run();
		int setupServer(int port);
		const std::string& getName() const;
		
		void acceptNewClient();
		bool handleClientRead(size_t i);
		void handleClientWrite();
		void sendToClient(int client_fd, const std::string& message);
		void processCommand(Client& client, const std::string& command);
		void tryAuthenticate(Client& client);
		
		void execPass(Client& client, const std::string& params);
		void execNick(Client& client, const std::string& params);
		void execUser(Client& client, const std::string& params);
		void execJoin(Client& client, const std::string& params);
		void execPrivmsg(Client& client, const std::string& params);
		void execInvite(Client& client, const std::string& params);
		void execTopic(Client &client, std::string &params);
		void execMode(Client& client, std::string& params);
		void execKick(Client& client, const std::string& params);
		void execQuit(Client& client, const std::string& params);
		void sendReply(Client& client, const std::string& msg);	
};

#endif
/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cacorrea <cacorrea@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/13 14:04:19 by sel-khao          #+#    #+#             */
/*   Updated: 2026/04/25 17:11:35 by cacorrea         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

# include "ft_irc.hpp"
# include "client.hpp"
# include "channel.hpp"

class Server
{
	private:
		std::string 					server_name;
		std::vector<struct pollfd>		poll_fds;
		std::vector<Client>				client_vect;
		std::map<std::string, Channel>	channels;
		int								socket_fd;
		bool							initialized;
		std::string						server_port;
		std::string						server_password;

	public:
		Server(const std::string& port, const std::string& password);
		~Server();
		void run();
		int setupServer(int port);
		bool isInitialized() const;
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

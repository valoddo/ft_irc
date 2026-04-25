/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ircbot.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cacorrea <cacorrea@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/23 13:00:27 by sel-khao          #+#    #+#             */
/*   Updated: 2026/04/25 17:19:05 by cacorrea         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef IRCBOT_HPP
# define IRCBOT_HPP

# include "ft_irc.hpp"

class IRCBot {
    private:
        int							sock_fd;
        std::string					server_ip;
        int							server_port;
        std::string					password;
        std::string					nickname;
        std::string					username;
        std::string					channel;
        std::string					read_buffer;
        std::vector<std::string>	quotes;
        std::string					sender;

		void                sendRaw(const std::string& msg);
        std::string         receiveLine(void);
		void                registerToServer(void);
		void                joinChannel(void);
		bool                handleServerMessage(const std::string& line);
		//void                sendHelp();
        void                sendHelp(const std::string& target);
		void                processCommand(const std::string& sender, const std::string& target, const std::string& cmd, const std::string& args);
		void                parseMessage(const std::string& line);
public:
	IRCBot(const std::string& ip, int port, const std::string& pass, const std::string& nick, const std::string& chan);
	~IRCBot(void);

	void                connectToServer(void);
	void                run(void);
};

#endif

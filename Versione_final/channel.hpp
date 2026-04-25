/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cacorrea <cacorrea@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/17 18:06:03 by vloddo            #+#    #+#             */
/*   Updated: 2026/04/25 17:25:46 by cacorrea         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
# define CHANNEL_HPP

# include "ft_irc.hpp"
# include "client.hpp"

class Channel
{
	private:
		std::string 					channel_name;
		std::string 					topic;
		std::string 					password;
		size_t 							user_limit;
		std::map<int, bool>				clients;
		std::vector<std::string>		invited;
		bool 							invite_only;
		bool 							topic_restricted;

		std::string						getChannelModes();
		
	public:
		Channel();
		Channel(const std::string& name);
		~Channel();

		const std::string& getName() const;
		const std::string& getTopic() const;
		const std::string& getPass() const;
		const bool& isInviteOnly() const;
		const bool& getTopicRestrict() const;

		void setName(const std::string& name);
		void setTopic(const std::string& topic);
		void setPass(const std::string& pass);
		void setClientOp(Client& commander, const std::vector<Client>& client_vect, std::string targetName, char flagSign);

		// Utility methods
		bool isMember(Client& client) const;
		bool isOperator(Client& client) const;
		bool isInvited(Client& client) const;
		void broadcast(const std::string& message, const std::vector<Client>& client_vect, int exclude_fd = -1);
		void sendToClient(Client& client, const std::string& message);
		
		// Public methods for commands
		void processJoin(Client& client, const std::vector<Client>& client_vect, const std::string& pass);
		void processInvite(Client& inviter, Client& target);
		void processMode(Client& client, const std::vector<Client>& client_vect, const std::vector<std::string>& param);
		void processKick(Client& kicker, Client& target, const std::vector<Client>& client_vect, const std::string& reason);
		void processQuit(Client& client, const std::vector<Client>& client_vect, const std::string& quitMessage);
		void removeClient(Client& client);
};

#endif

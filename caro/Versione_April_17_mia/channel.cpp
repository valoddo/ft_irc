/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cacorrea <cacorrea@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/17 22:14:52 by vloddo            #+#    #+#             */
/*   Updated: 2026/04/18 20:21:51 by cacorrea         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "channel.hpp"

Channel::Channel()
{
	channel_name = "";
    password = "";             // DEFAULT: vuota (nessuna password)
    user_limit = 0;              // DEFAULT: 0 (nessun limite)
	invite_only = false;        // DEFAULT: false (chiunque può entrare)
    topic_restricted = false;   // DEFAULT: false (tutti possono cambiare topic)
}

Channel::Channel(const std::string& name)
{
	channel_name = name;
    password = "";             // DEFAULT: vuota (nessuna password)
    user_limit = 0;              // DEFAULT: 0 (nessun limite)
	invite_only = false;        // DEFAULT: false (chiunque può entrare)
    topic_restricted = false;   // DEFAULT: false (tutti possono cambiare topic)
}

Channel::~Channel() {}

const std::string& Channel::getName() const
{
    return channel_name;
}

const std::string& Channel::getTopic() const
{
    return topic;
}

void	Channel::setTopic(const std::string& _topic)
{
    topic = _topic;
}

bool Channel::isMember(Client* client) const
{
    return (clients.find(client) != clients.end());
}


bool Channel::isOperator(Client* client) const
{
    std::map<Client*, bool>::const_iterator it = clients.find(client);
    if (it != clients.end())
        return it->second;  // true = operator, false = normal user
    return false;
}

bool Channel::isTopicRestricted() const
{
	return topic_restricted;
}

bool Channel::isInviteOnly() const
{
	return invite_only;	
}

void Channel::broadcast(const std::string& message, Client* exclude)
{
    for (std::map<Client*, bool>::iterator it = clients.begin(); it != clients.end(); ++it)
    {
        if (exclude == NULL || it->first != exclude)
        {
            sendToClient(*it->first, message);
        }
    }
}

void Channel::sendToClient(Client& client, const std::string& message)
{
    client.getWriteBuffer() += message + "\r\n";
}




		// Metodi pubblici per i comandi IRC
		void processJoin(Client& client, const std::string& password = "");
		void processPrivmsg(Client& sender, const std::string& message);
		void processInvite(Client& inviter, Client& target);
		void processTopic(Client& setter, const std::string& newTopic);
		void processMode(Client& changer, const std::string& mode, const std::string& param = "");
		void processKick(Client& kicker, Client& target, const std::string& reason = "");
		void processQuit(Client& client, const std::string& quitMessage);


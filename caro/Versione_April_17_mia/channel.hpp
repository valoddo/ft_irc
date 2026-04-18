/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cacorrea <cacorrea@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/17 18:06:03 by vloddo            #+#    #+#             */
/*   Updated: 2026/04/18 21:21:59 by cacorrea         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include "client.hpp"

#include <iostream>
#include <unistd.h>
#include <string.h>
#include <vector>
#include <map>

class Channel
{
	private:
		std::string 					channel_name;
		std::string 					topic;
		std::string 					password;		// per modalita' +k: se +k il canale e protetto con una password e puo entrare chi la conosce, se non ce +k non e richiesta la password
		size_t 							user_limit;		// per modalita' +l: se nessun +l non c'e limite di utenti (0 = nessun limite) 
		std::map<Client*, bool>			clients;		// true=operator, false=normal user
		std::vector<std::string>		invited;		// nickname invitati (per modalita' +i)
		bool 							invite_only;	// modalita' +i: se true(+i) nessuno puo entrare con JOIN se non invitato, se false(nessun +i) chiunque puo fare JOIN 
		bool 							topic_restricted; // modalita' +t: se true(+t) solo utenti con @ possono usare TOPIC per cambiare topic, se false(nessun +t) tutti possono cambiare topic

	public:
		Channel();
		Channel(const std::string& name);
		~Channel();

		const	std::string& getName() const;
		const	std::string& getTopic() const;
		
		void	setTopic(const std::string& topic);

		// Metodi di utility
		bool isMember(Client* client) const;
		bool isOperator(Client* client) const;
		bool isTopicRestricted() const;
		bool isInviteOnly() const;
		void broadcast(const std::string& message, Client* exclude = NULL);
		void sendToClient(Client& client, const std::string& message);
		
		// Metodi pubblici per i comandi IRC
		void processJoin(Client& client, const std::string& password = "");
		void processPrivmsg(Client& sender, const std::string& message);
		void processInvite(Client& inviter, Client& target);
		void processTopic(Client& setter, const std::string& newTopic);
		void processMode(Client& changer, const std::string& mode, const std::string& param = "");
		void processKick(Client& kicker, Client& target, const std::string& reason = "");
		void processQuit(Client& client, const std::string& quitMessage);
};

#endif
/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cacorrea <cacorrea@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/17 18:06:03 by vloddo            #+#    #+#             */
/*   Updated: 2026/04/21 15:03:02 by cacorrea         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
# define CHANNEL_HPP

# include "client.hpp"

# include <iostream>
# include <unistd.h>
# include <string.h>
# include <vector>
# include <map>
# include <sstream>

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

		std::string						getModes();
		
	public:
		Channel();
		Channel(const std::string& name);
		~Channel();

		const std::string& getName() const;
		const std::string& getTopic() const;
		const std::string& getPass() const;
		const bool& getInviteOnly() const; // ritorna valore booleano se il channel e invite only o no
		const bool& getTopicRestrict() const;

		void setName(const std::string& name);
		void setTopic(const std::string& topic);
		void setPass(const std::string& pass);
		void setClientOp(Client& commander, std::string targetName, char flagSign);

		// Metodi di utility
		bool isMember(Client& client) const;
		bool isOperator(Client& client) const;
		bool isInvited(Client& client) const;  //ritorna valore booleano se il targetnick(nickname invitato) fa parte della lista di invitati o no
		void broadcast(const std::string& message, Client* exclude = NULL);
		void sendToClient(Client& client, const std::string& message);
		
		// Metodi pubblici per i comandi IRC
		void processJoin(Client& client, const std::string& pass);  //con void processJoin(Client& client, const std::string& password = "") di default se il secondo parametro non esiste, viene impostato a vuoto
		void processInvite(Client& inviter, const std::string& target);
		void processTopic(Client& setter, const std::string& newTopic);
		void processMode(Client& client, const std::vector<std::string>& param);
		void processKick(Client& kicker, Client& target, const std::string& reason);
		void processQuit(Client& client, const std::string& quitMessage);
		void removeClient(Client& client);
};

#endif
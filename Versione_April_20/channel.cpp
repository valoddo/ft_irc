/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sel-khao <sel-khao@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/17 22:14:52 by vloddo            #+#    #+#             */
/*   Updated: 2026/04/19 19:58:29 by sel-khao         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "channel.hpp"

void Channel::removeClient(Client& client){
	clients.erase(&client);
}

Channel::Channel()
{
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

const std::string& Channel::getName() const {return(channel_name);}

const std::string& Channel::getTopic() const {return(topic);}

const std::string& Channel::getPass() const {return(password);}

const bool& Channel::getInviteOnly() const {return(invite_only);} 

const bool& Channel::getTopicRestrict() const{return(topic_restricted);}

void Channel::setName(const std::string& name) {channel_name = name;}
		
void Channel::setTopic(const std::string& topic) {this->topic = topic;}

void Channel::setPass(const std::string& pass) {password = pass;}

bool Channel::isMember(Client& client) const
{
    if(clients.find(&client) != clients.end()) //cerca client nel contenitore di clients, clients.end() indica la posizione dell'iteratore alla fine della lista
            return(true); //se l'iteratore e diverso da clients.end() allora vuol dire che il clent e stato trovato ed e quindi membro della channel
    else
        return(false);
}

bool Channel::isOperator(Client& client) const
{
    std::map<Client*, bool>::const_iterator it = clients.find(&client); // dichiaro l'iteratore per controllare nel container map
    if (it != clients.end())
        return it->second;  // true = operator, false = normal user
    return false;
}

bool Channel::isInvited(Client& client) const
{
    for(size_t i = 0; i < invited.size(); i++)
    {
        if(invited[i] == client.getNick())
            return(true);
    }
    return(false);
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


void Channel::processJoin(Client& client, const std::string& pass) //con void processJoin(Client& client, const std::string& password = "") di default se il secondo parametro non esiste, viene impostato a vuoto
{
    if(pass!= password){
        client.getWriteBuffer() += "475 " + client.getNick() + " " + channel_name + " :Bad channel key\r\n";
        return;        
    }
    if(getInviteOnly() && !isInvited(client)){
        client.getWriteBuffer() += "473 " + client.getNick() + " " + channel_name + " :Cannot join channel (+i)\r\n";
        return;        
    }
    if(isMember(client))
        return;
    else
        clients[&client] = false;
    
}



		
        
void Channel::processInvite(Client& inviter, const std::string& target)
{
    if(getInviteOnly() && !isOperator(inviter))
    {
        inviter.getWriteBuffer() += "482 " + getName() + ":You’re not channel operator\r\n";
        return;        
    }
    invited.push_back(target);
}
        
    
void Channel::processPrivmsg(Client& sender, const std::string& message){
    std::string fullMsg = ":" + sender.getPrefix() + " PRIVMSG " + channel_name + " :" + message + "\r\n";
    broadcast(fullMsg, &sender);//a tutti tranne mittente
}

/*
di cosa parla il canale?
TOPIC #canale (mostra topic corrente)
TOPIC #canale :Nuovo topic (per cambiare topic)

setter = il cliente vuole vedere o cambiare topic
newTpocic = new topic, or empty if just wanna see
*/
void Channel::processTopic(Client& setter, const std::string& newTopic){
	if (newTopic.empty()){//TOPIC #canale. 
		if (topic.empty())
			setter.getWriteBuffer() += " 331 " + setter.getNick() + " " + channel_name + " :No topic is set\r\n";
			//331 theres no topic impostato
		else
			setter.getWriteBuffer() += "332 " + setter.getNick() + " " + channel_name + " :" + topic + "\r\n";
			//mostra topic corrente
		return ;
	}
	//se not empty then he wanna see topiuc
	if (!isMember(setter)){
        setter.getWriteBuffer() += "442 " + setter.getNick() + " " + channel_name + " :You're not on that channel\r\n";
        return ;
	}
	//verifico i permessi, if modalita' +t, topic_restricted = modalita' +t del canale. se +t only operatore, if -t everyone
    if (topic_restricted && !isOperator(setter)){
        setter.getWriteBuffer() += "482 " + setter.getNick() + " " + channel_name + " :You're not a channel operator\r\n";
        return ;
    }
	topic = newTopic;
    // Broadcast del cambiamento
    std::string msg = ":" + setter.getPrefix() + " TOPIC " + channel_name + " :" + newTopic + "\r\n";
    broadcast(msg, NULL);
}

void Channel::processMode(Client& changer, const std::string& mode, const std::string& param){
    (void)changer;
    (void)mode;
    (void)param;
    
}

void Channel::processKick(Client& kicker, Client& target, const std::string& reason){
    if (!isOperator(kicker)){
        kicker.getWriteBuffer() += "482 " + channel_name + " :You're not a channel operator\r\n";
        return;
    }
    if (!isMember(target)){
        kicker.getWriteBuffer() += "441 " + target.getNick() + " " + channel_name + " :They aren't on that channel\r\n";
        return;
    }
    std::string kickMsg = ":" + kicker.getPrefix() + " KICK " + channel_name + " " + target.getNick() + " :" + reason + "\r\n";
    broadcast(kickMsg, NULL);
    removeClient(target);
}

void Channel::processQuit(Client& client, const std::string& quitMessage){
    if (!isMember(client))//if client not member do nothing
        return ;
    std::string quitMsg = ":" + client.getPrefix() + " QUIT :" + quitMessage + "\r\n";
    broadcast(quitMsg, NULL);
    removeClient(client);
}


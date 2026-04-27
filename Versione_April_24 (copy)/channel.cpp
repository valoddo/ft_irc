/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cacorrea <cacorrea@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/17 22:14:52 by vloddo            #+#    #+#             */
/*   Updated: 2026/04/24 13:15:26 by cacorrea         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "channel.hpp"

void Channel::removeClient(Client& client){
	clients.erase(&client);
}

Channel::Channel()
{
	password = "";
	user_limit = 0;
	invite_only = false;
	topic_restricted = false;
}

Channel::Channel(const std::string& name)
{
	channel_name = name;
	password = "";
	user_limit = 0;
	invite_only = false;
	topic_restricted = false;
}

Channel::~Channel() {}

const std::string& Channel::getName() const {return(channel_name);}

const std::string& Channel::getTopic() const {return(topic);}

const std::string& Channel::getPass() const {return(password);}

const bool& Channel::isInviteOnly() const {return(invite_only);} 

const bool& Channel::getTopicRestrict() const{return(topic_restricted);}

void Channel::setName(const std::string& name) {channel_name = name;}
		
void Channel::setTopic(const std::string& topic) {this->topic = topic;}

void Channel::setPass(const std::string& pass) {password = pass;}

bool Channel::isMember(Client& client) const
{
	if(clients.find(&client) != clients.end())
		return(true);
	else
		return(false);
}

bool Channel::isOperator(Client& client) const
{
	std::map<Client*, bool>::const_iterator it = clients.find(&client);
	if (it != clients.end())
		return it->second;
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

//If the second parameter is missing, it defaults to an empty value
void Channel::processJoin(Client& client, const std::string& pass, const std::string& server_name)
{
	if(pass!= password){
		client.getWriteBuffer() += "475 " + client.getNick() + " " + channel_name + " :Bad channel key\r\n";
		return;        
	}
	if(isInviteOnly() && !isInvited(client)){
		client.getWriteBuffer() += "473 " + client.getNick() + " " + channel_name + " :Cannot join channel (+i)\r\n";
		return;        
	}

	if(user_limit != 0 && clients.size() >= user_limit)
	{
		client.getWriteBuffer() += "471 " + client.getNick() + " " + channel_name + " :Cannot join channel (+l)\r\n";
		return;  	
	}
	if(isMember(client)){
		client.getWriteBuffer() += ":" + server_name + " 443 " + client.getNick() + " #" + getName() + " :is already on channel\r\n";
		return;
	}
	else{
		if(clients.empty())
			clients[&client] = true;
		else	
			clients[&client] = false;
		broadcast(":" + client.getPrefix() + " JOIN :" + getName(), NULL);
		client.getWriteBuffer() += ":" + server_name + " 353 " + client.getNick() + " = " + getName() + " :@" + client.getNick() + "\r\n";
		client.getWriteBuffer() += ":" + server_name + " 366 " + client.getNick() + " " + getName() + " :End of /NAMES list\r\n";
	}
}
		
void Channel::processInvite(Client& inviter, Client& target)
{
	if(isInviteOnly() && !isOperator(inviter))
	{
		inviter.getWriteBuffer() += "482 " + getName() + ":You’re not channel operator\r\n";
		return;        
	}
	invited.push_back(target.getNick());
	inviter.getWriteBuffer() += "341 " + inviter.getNick() + " " + target.getNick() + " :" + getName() + "\r\n";
	target.getWriteBuffer() += ":" + inviter.getPrefix() + " INVITE " + target.getNick() + " :" + getName() + "\r\n";
}

// Sets or removes operator status (+o / -o) for a target client in the channel.
// Broadcasts the MODE change; sends an error if the target nick is not found.
// flagSign is the value of the flag to be set
void Channel::setClientOp(Client& commander, std::string targetName, char flagSign)
{
	std::map<Client*, bool>::iterator it;
	
	for (it = clients.begin(); it != clients.end(); ++it)
	{
		if (it->first->getNick() == targetName)
		{
			it->second = (flagSign == '+');
			std::string sign(1, flagSign);
			std::string msg = ":" + commander.getPrefix() + " MODE " 
				+ channel_name + " " + sign + "o " + targetName + "\r\n";

			broadcast(msg, NULL);
			return;
		}
	}
	commander.getWriteBuffer() +=  commander.getNick() + " " + targetName + 
		" :No such nick\r\n";
}

void Channel::processMode(Client& client, const std::vector<std::string>& param)
{
	char	flagSign = '+';
	size_t	index = 1;
	
	if (param.size() == 1)
	{
		std::string modes = getChannelModes();
		if (modes.find('l') != std::string::npos)
		{
			std::stringstream ss;
			ss << user_limit;
			modes += " " + ss.str();
		}
		if (isMember(client) && (modes.find('k') != std::string::npos))
			modes += " " + getPass();
		client.getWriteBuffer() += "324 " + client.getNick() + " " + channel_name + " :" + modes + "\r\n";
		return;
	}
	std::string modes = param[1];
	for (size_t i = 0; i < modes.length(); i++)
	{
		char	c = modes[i];
		if (c == '+' || c == '-')
		{
			flagSign = c;
			continue;			
		}
		if (c == 'i' || c == 't' || (flagSign == '-' && (c == 'k' || c == 'l')))
		{
			if (!isOperator(client))
			{
				client.getWriteBuffer() += "482 " + client.getNick() + " " + channel_name + 
					" :You're not a channel operator\r\n";
				continue ;
			}
			else
			{
				if (c == 'i' && flagSign == '+' && !invite_only)
					invite_only = true;
				else if (c == 'i' && flagSign == '-' && invite_only)
					invite_only = false;
				else if (c == 't' && flagSign == '+' && !topic_restricted)
					topic_restricted = true;
				else if (c == 't' && flagSign == '-' && topic_restricted)
					topic_restricted = false;
				else if (c == 'k')
					password = "";
				else if (c == 'l')
					user_limit = 0;
				std::string	flag(1, flagSign);
				flag.append(1, c);
				std::string	msg = ":" + client.getPrefix() + " MODE " + channel_name + 
					" " + flag;
				broadcast(msg, NULL);
			}	
		}
		else if (c == 'o' || c == 'k' || c == 'l')
		{
			if (++index >= param.size())
			{
				client.getWriteBuffer() += "461 " + client.getNick() + " MODE :Not enough parameters\r\n";
				continue ;
			}
			else if(isOperator(client))
			{
				if (c == 'k' && flagSign == '+')
				{
					password = param[index];
					std::string flags = " +k ";
					std::string	msg = ":" + client.getPrefix() + " MODE " + channel_name + 
						flags + param[index];
					broadcast(msg, NULL);
				}
				else if (c == 'l' && flagSign == '+')
				{
					std::stringstream ss(param[index]);
					size_t value;
					if (!(ss >> value) || !ss.eof())
						continue ;
					user_limit = value;
					std::string flags = " +l ";
					std::string	msg = ":" + client.getPrefix() + " MODE " + channel_name + 
						flags + param[index] + "\r\n";
					broadcast(msg, NULL);
				}
				if (c == 'o')	
				{
					setClientOp(client, param[index], flagSign);
				}
			}
			else
				client.getWriteBuffer() += "482 " + client.getNick() + " " + channel_name + " :You're not a channel operator\r\n";
		}
		else
		{
			client.getWriteBuffer() += "472 " + client.getNick() + " " + c + 
			" :is an unknown mode char to me\r\n";
		}
	} 
}

std::string	Channel::getChannelModes()
{
	std::string modes;
	if (invite_only)
		modes.append("i");
	if (topic_restricted)
		modes.append("t");
	if (user_limit > 0)
		modes.append("l");
	if (!password.empty())
		modes.append("k");
	if (!modes.empty())
		modes.insert(0, 1, '+');
	return modes;
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


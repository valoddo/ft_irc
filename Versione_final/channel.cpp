/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cacorrea <cacorrea@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/17 22:14:52 by vloddo            #+#    #+#             */
/*   Updated: 2026/04/25 17:30:28 by cacorrea         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "channel.hpp"

static Client* findClientByFd(const std::vector<Client>& client_vect, int fd)
{
	for (size_t i = 1; i < client_vect.size(); ++i)
	{
		if (client_vect[i].getClientFd() == fd)
			return const_cast<Client*>(&client_vect[i]);
	}
	return NULL;
}

void Channel::removeClient(Client& client){
	clients.erase(client.getClientFd());
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
	if(clients.find(client.getClientFd()) != clients.end())
		return(true);
	else
		return(false);
}

bool Channel::isOperator(Client& client) const
{
	std::map<int, bool>::const_iterator it = clients.find(client.getClientFd());
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

void Channel::broadcast(const std::string& message, const std::vector<Client>& client_vect, int exclude_fd)
{
	for (std::map<int, bool>::iterator it = clients.begin(); it != clients.end(); ++it)
	{
		if (exclude_fd != -1 && it->first == exclude_fd)
			continue;
		Client* target = findClientByFd(client_vect, it->first);
		if (target != NULL)
			sendToClient(*target, message);
	}
}

void Channel::sendToClient(Client& client, const std::string& message)
{
	client.getWriteBuffer() += message + "\r\n";
}

//If the second parameter is missing, it defaults to an empty value
void Channel::processJoin(Client& client, const std::vector<Client>& client_vect, const std::string& pass)
{
	if(pass!= password){
		client.getWriteBuffer() += ":" SERVER_NAME " 475 " + client.getNick() + " " + channel_name + " :Bad channel key\r\n";
		return;        
	}
	if(isInviteOnly() && !isInvited(client)){
		client.getWriteBuffer() += ":" SERVER_NAME " 473 " + client.getNick() + " " + channel_name + " :Cannot join channel (+i)\r\n";
		return;        
	}

	if(user_limit != 0 && clients.size() >= user_limit)
	{
		client.getWriteBuffer() += ":" SERVER_NAME " 471 " + client.getNick() + " " + channel_name + " :Cannot join channel (+l)\r\n";
		return;  	
	}
	if(isMember(client)){
		client.getWriteBuffer() += ":" SERVER_NAME " 443 " + client.getNick() + " #" + getName() + " :is already on channel\r\n";
		return;
	}
	else{
		if(clients.empty())
			clients[client.getClientFd()] = true;
		else	
			clients[client.getClientFd()] = false;
		broadcast(":" + client.getPrefix() + " JOIN :" + getName(), client_vect);
		client.getWriteBuffer() += ":" SERVER_NAME " 353 " + client.getNick() + " = " + getName() + " :@" + client.getNick() + "\r\n";
		client.getWriteBuffer() += ":" SERVER_NAME " 366 " + client.getNick() + " " + getName() + " :End of /NAMES list\r\n";
	}
}
		
void Channel::processInvite(Client& inviter, Client& target)
{
	if(isInviteOnly() && !isOperator(inviter))
	{
		inviter.getWriteBuffer() += ":" SERVER_NAME " 482 " + inviter.getNick() + " " + getName() + " :You're not a channel operator\r\n";
		return;        
	}
	invited.push_back(target.getNick());
	inviter.getWriteBuffer() += ":" SERVER_NAME " 341 " + inviter.getNick() + " " + target.getNick() + " " + getName() + "\r\n";
	target.getWriteBuffer() += ":" + inviter.getPrefix() + " INVITE " + target.getNick() + " :" + getName() + "\r\n";
}

// Sets or removes operator status (+o / -o) for a target client in the channel.
// Broadcasts the MODE change; sends an error if the target nick is not found.
// flagSign is the value of the flag to be set
void Channel::setClientOp(Client& commander, const std::vector<Client>& client_vect, std::string targetName, char flagSign)
{
	for (size_t i = 1; i < client_vect.size(); ++i)
	{
		if (client_vect[i].getNick() == targetName)
		{
			std::map<int, bool>::iterator it = clients.find(client_vect[i].getClientFd());
			if (it == clients.end())
				break;
			it->second = (flagSign == '+');
			std::string sign(1, flagSign);
			std::string msg = ":" + commander.getPrefix() + " MODE " 
				+ channel_name + " " + sign + "o " + targetName;

			broadcast(msg, client_vect);
			return;
		}
	}
	commander.getWriteBuffer() +=  ":" SERVER_NAME " 401 " + commander.getNick() + " " + targetName + 
		" :No such nick\r\n";
}

void Channel::processMode(Client& client, const std::vector<Client>& client_vect, const std::vector<std::string>& param)
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
		client.getWriteBuffer() += ":" SERVER_NAME " 324 " + client.getNick() + " " + channel_name + " :" + modes + "\r\n";
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
				client.getWriteBuffer() += ":" SERVER_NAME " 482 " + client.getNick() + " " + channel_name + 
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
				broadcast(msg, client_vect);
			}	
		}
		else if (c == 'o' || c == 'k' || c == 'l')
		{
			if (++index >= param.size())
			{
				client.getWriteBuffer() += ":" SERVER_NAME " 461 " + client.getNick() + " MODE :Not enough parameters\r\n";
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
					broadcast(msg, client_vect);
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
					broadcast(msg, client_vect);
				}
				if (c == 'o')	
				{
					setClientOp(client, client_vect, param[index], flagSign);
				}
			}
			else
				client.getWriteBuffer() += ":" SERVER_NAME " 482 " + client.getNick() + " " + channel_name + " :You're not a channel operator\r\n";
		}
		else
		{
			client.getWriteBuffer() += ":" SERVER_NAME " 472 " + client.getNick() + " " + c + 
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

void Channel::processKick(Client& kicker, Client& target, const std::vector<Client>& client_vect, const std::string& reason){
	if (!isOperator(kicker)){
		kicker.getWriteBuffer() += ":" SERVER_NAME " 482 " + kicker.getNick() + " " + channel_name + " :You're not a channel operator\r\n";
		return;
	}
	if (!isMember(target)){
		kicker.getWriteBuffer() += ":" SERVER_NAME " 441 " + kicker.getNick() + " " + target.getNick() + " " + channel_name + " :They aren't on that channel\r\n";
		return;
	}
	std::string kickMsg = ":" + kicker.getPrefix() + " KICK " + channel_name + " " + target.getNick() + " :" + reason + "\r\n";
	broadcast(kickMsg, client_vect);
	removeClient(target);
}

void Channel::processQuit(Client& client, const std::vector<Client>& client_vect, const std::string& quitMessage){
	if (!isMember(client))//if client not member do nothing
		return ;
	std::string quitMsg = ":" + client.getPrefix() + " QUIT :" + quitMessage + "\r\n";
	broadcast(quitMsg, client_vect);
	removeClient(client);
}

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vloddo <vloddo@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/17 22:14:52 by vloddo            #+#    #+#             */
/*   Updated: 2026/04/20 21:13:58 by vloddo           ###   ########.fr       */
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
    if(isMember(client)){
        client.getWriteBuffer() += ":inception 443 " + client.getNick() + " #" + getName() + " :is already on channel\r\n";
        return;
    }
    else{
        clients[&client] = false;
        broadcast(":" + client.getPrefix() + " JOIN :" + getName(), NULL);
        client.getWriteBuffer() += ":inception 353 " + client.getNick() + " = " + getName() + " :@" + client.getNick() + "\r\n";
        client.getWriteBuffer() += ":inception 366 " + client.getNick() + " " + getName() + " :End of /NAMES list\r\n";
    }
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

/*
di cosa parla il canale?
TOPIC #canale (mostra topic corrente)
TOPIC #canale :Nuovo topic (per cambiare topic)

setter = il cliente vuole vedere o cambiare topic
newTpocic = new topic, or empty if just wanna see
*/
//482 ERR_CHANOPRIVSNEEDED
static void	wrErrChnOpNeeded(Client& client, std::string ch_name)
{
	client.getWriteBuffer() += "482 " + client.getNick() + " " + ch_name + 
	" :You're not a channel operator\r\n";
	return ;
}

//devo settare la flag se un client e operatore o meno, paramteri:
void Channel::setClientOp(Client& commander, std::string targetName, char flagSign)
{
    std::map<Client*, bool>::iterator it;
    
    for (it = clients.begin(); it != clients.end(); ++it)
    {
        if (it->first->getNick() == targetName)
        {
            it->second = (flagSign == '+');//se la condizione e vera la booleana e vera
            std::string sign(1, flagSign);//costructore di string
            std::string msg = ":" + commander.getPrefix() + " MODE " 
                + channel_name + " " + sign + "o " + targetName + "\r\n";

            broadcast(msg, NULL);
            return;
        }
    }
	//>> :platinum.libera.chat 401 cacorreaa pallino :No such nick/channel
    commander.getWriteBuffer() +=  commander.getNick() + " " + targetName + 
		" :No such nick\r\n";//manca : servername
}

//param[0]=channel; param[1]=+to
void Channel::processMode(Client& client, const std::vector<std::string>& param)
{
    char	flagSign = '+';//salva se e + o - l'ultimo valore
    size_t	index = 1;//flags
	
	if (param.size() == 1)//324 RPL_CHANNELMODEIS
	{
		std::string modes = getModes();
		if (modes.find('l') != std::string::npos)
		{
			std::stringstream ss;
			ss << user_limit;
			modes += " " + ss.str();
		}
		if (isMember(client) && (modes.find('k') != std::string::npos))
			modes += " " + getPass();
		client.getWriteBuffer() += "324 " + client.getNick() + " " + channel_name + " "
		+ modes + "\r\n";
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
		if (c == 'i' || c == 't' || (flagSign == '-' && (c == 'k' || c == 'l')))//no params needed (maybe helper function1)
		{
			if (!isOperator(client))
			{
				wrErrChnOpNeeded(client, channel_name);
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
//>> @time=2026-04-20T16:19:47.788Z :cacorreaa!~carolina@185.30.66.235 MODE #my42irc +i
				std::string	msg = ":" + client.getPrefix() + " MODE " + channel_name + 
					" " + flag + "\r\n";//manca il time all'inizio solo nel sender
        		broadcast(msg, NULL);
			}	
		}
		else if (c == 'o' || c == 'k' || c == 'l')//parameters needed (maybe helper function2)
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
//>> @time=2026-04-20T16:22:57.933Z :cacorreaa!~carolina@185.30.66.235 MODE #my42irc +k pwpw
					std::string flags = " +k ";
					std::string	msg = ":" + client.getPrefix() + " MODE " + channel_name + 
						flags + param[index] + "\r\n";//manca time ini sender
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
				wrErrChnOpNeeded(client, channel_name);
		}
		else//472  ERR_UNKNOWNMODE
		{
			client.getWriteBuffer() += "472 " + client.getNick() + " " + c + 
			" :is an unknown mode char to me\r\n";
		}
	} 
}

std::string	Channel::getModes()
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


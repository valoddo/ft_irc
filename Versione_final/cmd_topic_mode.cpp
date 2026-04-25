/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cmd_topic_mode.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cacorrea <cacorrea@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/24 10:28:08 by cacorrea          #+#    #+#             */
/*   Updated: 2026/04/25 17:40:08 by cacorrea         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.hpp"

static  std::vector<std::string> extractTokens(const std::string& input)
{
	std::vector<std::string> 	tokens;
	std::string					token;
	std::stringstream			ss(input);

	while (ss >> token)
	{
		tokens.push_back(token);
	}
	return (tokens);
}

static  std::vector<std::string>    littelParsing(std::string& parameters)
{
	size_t						pos;
	std::string					msg;
	bool						hasMsg = false;
	std::vector<std::string>    result;

	pos = parameters.find(":");
	if (pos != std::string::npos)
	{
		msg = parameters.substr(pos + 1);
		parameters = parameters.substr(0, pos);
		hasMsg = true;
	}
	result = extractTokens(parameters);
	if (hasMsg)
		result.push_back(msg);
	return (result);
}

static void	printTopic(Client& client, Channel& channel)
{
	if (channel.getTopic().empty())
	{
		client.getWriteBuffer() += ":" SERVER_NAME " 331 " + client.getNick() 
		+ " " + channel.getName() + " :No topic is set\r\n";
	}
	else
	{
		client.getWriteBuffer() += ":" SERVER_NAME " 332 " + client.getNick() 
		+ " " + channel.getName() + " :" + channel.getTopic() + "\r\n";
	}
}

static bool	canChangeTopic(Client& client, Channel& channel)
{
	if (!channel.isMember(client))
	{
		client.getWriteBuffer() += ":" SERVER_NAME " 442 " + client.getNick() 
			+ " " + channel.getName() + " :You're not on that channel\r\n";
		return (false);
	}
	if (channel.getTopicRestrict() && !(channel.isOperator(client)))
	{
		client.getWriteBuffer() += ":" SERVER_NAME " 482 " + client.getNick() 
			+ " " + channel.getName() + " :You're not a channel operator\r\n";
		return (false);
	}
	return (true);
}

// command use: TOPIC <#channel> [<topic>]
void    Server::execTopic(Client &client, std::string &params) 
{
	std::vector<std::string>    parameters;

	parameters = littelParsing(params);
	if (parameters.empty())
	{
		client.getWriteBuffer() += ":" + getName() + " 461 " + client.getNick() 
			+ " TOPIC :Not enough parameters\r\n";
		return;
	}
	std::string channelName = parameters[0];

	if (channelName.empty() || channelName[0] != '#')
	{
		client.getWriteBuffer() += ":" + getName() + " 403 " + client.getNick() 
			+ " " + channelName + " :No such channel\r\n";
		return;
	}
	std::map<std::string, Channel>::iterator it = channels.find(channelName);
	if (it == channels.end())
	{
		client.getWriteBuffer() += ":" + getName() + " 403 " + client.getNick() 
			+ " " + channelName + " :No such channel\r\n";
		return;
	}
	Channel	&ch = it->second;
	
	if (parameters.size() == 1)
	{
		printTopic(client, ch);
		return ;
	}
	else if (parameters.size() > 1)
	{
		if (!canChangeTopic(client, ch))
			return;
		ch.setTopic(parameters[1]);
		std::string	msg = ":" + client.getPrefix() + " TOPIC " + channelName + " :" + ch.getTopic();
		ch.broadcast(msg, client_vect);
	}
}

// command use: MODE <channel> [modes] [parameters]
void Server::execMode(Client& client, std::string& params)
{
	std::vector<std::string>    parameters;

	parameters = littelParsing(params);
	if (params.empty())
	{
		client.getWriteBuffer() += ":" + getName() + " 461 " + client.getNick() 
			+ " MODE :Not enough parameters\r\n";
		return;
	}
	if (parameters.size() >= 1)
	{
		std::string channelName = parameters[0];
		if (channelName.empty() || channelName[0] != '#')
		{
			client.getWriteBuffer() += ":" + getName() + " 403 " + client.getNick() 
				+ " " + channelName + " :No such channel\r\n";
			return;
		}
		std::map<std::string, Channel>::iterator it = channels.find(channelName);
		if (it == channels.end())
		{
			client.getWriteBuffer() += ":" + getName() + " 403 " + client.getNick()
				+ " " + channelName + " :No such channel\r\n";
			return;
		}
		Channel& ch = it->second;
		ch.processMode(client, client_vect, parameters);
	}
} 

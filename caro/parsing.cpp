/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cacorrea <cacorrea@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/16 18:07:10 by cacorrea          #+#    #+#             */
/*   Updated: 2026/04/17 10:15:01 by cacorrea         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <sstream>
#include <cstdlib>
#include <vector>

typedef struct s_command
{
	std::string					cmd;
	std::vector<std::string>	args;
}			t_command;

/* std::vector<std::string> extractTokens(const std::string& input)
{
	std::vector<std::string> tokens;

	size_t pos = 0;
	size_t start;
	size_t end;

	while (pos < input.length())
	{
		start = input.find_first_not_of(" \t", pos);
		if (start == std::string::npos)
			break;
		end = input.find_first_of(" \t", start);
		if (end == std::string::npos)
			end = input.length();
		tokens.push_back(input.substr(start, end - start));
		pos = end;
	}
	return (tokens);
} */

//da levare
void trimSpaces(std::string& s)
{
	size_t	start;
	size_t	end;
	size_t	len;

	start = s.find_first_not_of(" \t");
	if (start == std::string::npos)
	{
		s = "";
		return ;
	}
	end = s.find_last_not_of(" \t");
	len = end - start + 1;
	s = s.substr(start, len);
	return ;
}

std::vector<std::string> extractTokens(const std::string& input)
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

void printCompleteCmd(t_command result)
{
	std::cout << result.cmd;
	for (std::vector<std::string>::iterator it = result.args.begin();
		it != result.args.end(); ++it)
	{
		std::cout << ", " << *it;
	}
	std::cout << std::endl;
	
}

t_command	parse(std::string	input)
{
	t_command	result;
	size_t		pos;
	std::string	parameters;
	std::string	msg;
	bool		hasMsg = false;
	
	pos =  input.find_first_of(" \t");
	result.cmd = input.substr(0, pos);
	if (pos == std::string::npos)
		return (result);
	parameters = input.substr(pos + 1);
	pos = parameters.find(":");
	if (pos != std::string::npos)
	{//divide in due eliminando i :
		msg = parameters.substr(pos + 1);
		//trimSpaces(msg);//da levare
		parameters = parameters.substr(0, pos);
		hasMsg = true;
	}
	result.args = extractTokens(parameters);
	if (hasMsg)
		result.args.push_back(msg);
	printCompleteCmd(result);//da levare
	return (result);
}

int	main(int ac, char **av)
{
	if (ac == 2)
	{
		std::string	input = av[1];
		t_command	command;
		
		command = parse(input);
	}
	return (0);
}

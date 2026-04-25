/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cacorrea <cacorrea@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/15 13:34:58 by sel-khao          #+#    #+#             */
/*   Updated: 2026/04/25 17:09:43 by cacorrea         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "ft_irc.hpp"

class Client
{
	private:
		int			client_fd;
		std::string	client_ip;
		std::string pass_client;
		std::string nickname;
		std::string username;
		std::string read_buffer;
		std::string write_buffer;
		bool 		authenticated;

	public:
		Client(const Client& other);
		Client& operator=(const Client& other);
		Client();
		~Client();

		int 				getClientFd() const;
		const std::string&	getIp() const;
		const std::string&	getPass() const;
		const std::string&	getNick() const;
		const std::string&	getUser() const;
		std::string			getPrefix() const;

		bool isAuthenticated() const;

		void setClientFd(int fd);
		void setIP(const std::string&	ip);
		void setPass(const std::string&	pass);
		void setNick(const std::string&	nick);
		void setUser(const std::string&	user);
		void setAuthenticated(bool value);

		std::string&	getReadBuffer();
		std::string&	getWriteBuffer();
};

#endif

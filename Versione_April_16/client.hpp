/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: marvin <marvin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/15 13:34:58 by sel-khao          #+#    #+#             */
/*   Updated: 2026/04/16 01:31:02 by marvin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <iostream>
#include <string>
#include <unistd.h>
#include <cstring>
#include <sys/socket.h> //funzioni socket: socket(), connect(), send(), recv()
#include <arpa/inet.h>  // funzioni indirizzi IP: sockaddr_in, inet_addr, htons

class Client
{
	private:
		int			client_fd; // socket individuato come fd, ogni client ha un socket proprio
		std::string client_ip; // IP del client
		std::string nickname; // nome visibile in chat, unico nel server
		std::string username; // identificatore "tecnica" dell'utente
		std::string read_buffer; //buffer che serve in caso i messaggi arrivassero spezzati
		std::string write_buffer;
		bool 		authenticated; // variabile per chiarire se il client e autenticato


	public:
		Client();
		~Client();

		int getClientFd() const;
		const std::string& getIp() const;
		const std::string& getNick() const;
		const std::string& getUser() const;
		bool isAuthenticated() const;

		void setClientFd(int fd);
		void setIP(const std::string& ip);
		void setNick(const std::string& nick);
		void setUser(const std::string& user);
		void setAuthenticated(bool value);

		std::string& getReadBuffer();
		std::string& getWriteBuffer();
};

#endif


/*
NOTE:
Client = identità
Channel = permessi
Server = logica

ha senso mettere la flag Operator nel Channel perche un client puo essere operator in un channel ma client normale nell'altro
soluzione comune:
std::map<Client*, int> privileges;
 0 = User
 1 = Operator
 
 
 AUTENTICAZIONE DEL CLIENT:
 - Password: password del client deve essere uguale alla password del server 
 - Nickname: deve essere unico nel server (nickname identificativo nella chat)
 - Username: controllo che e stato ricevuto 
 
 
 */
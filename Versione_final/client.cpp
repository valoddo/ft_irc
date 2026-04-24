/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vloddo <vloddo@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/15 13:34:58 by sel-khao          #+#    #+#             */
/*   Updated: 2026/04/24 15:58:40 by vloddo           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "client.hpp"

Client::Client(const Client& other)
    : client_fd(other.client_fd), client_ip(other.client_ip),
      pass_client(other.pass_client), nickname(other.nickname),
      username(other.username), read_buffer(other.read_buffer),
      write_buffer(other.write_buffer), authenticated(other.authenticated)
{}

Client& Client::operator=(const Client& other)
{
    if (this != &other) {
        client_fd = other.client_fd;
        client_ip = other.client_ip;
        pass_client = other.pass_client;
        nickname = other.nickname;
        username = other.username;
        read_buffer = other.read_buffer;
        write_buffer = other.write_buffer;
        authenticated = other.authenticated;
    }
    return *this;
}

Client::Client(): client_fd(-1), authenticated(false){}

Client::~Client(){}

int Client::getClientFd() const{return(client_fd);}

const std::string& Client::getIp() const{return(client_ip);}

const std::string& Client::getPass() const{return(pass_client);}

const std::string& Client::getNick() const{return(nickname);}

const std::string& Client::getUser() const{return(username);}

std::string Client::getPrefix() const
{
	return (nickname + "!" + username +"@" + client_ip);
}

bool Client::isAuthenticated() const{return(authenticated);}

void Client::setClientFd(int fd){client_fd = fd;}

void Client::setIP(const std::string& ip){client_ip = ip;}

void Client::setPass(const std::string& pass){pass_client = pass;}

void Client::setNick(const std::string& nick){nickname = nick;}

void Client::setUser(const std::string& user){username = user;}

void Client::setAuthenticated(bool value){authenticated = value;}

std::string& Client::getReadBuffer(){return(read_buffer);}

std::string& Client::getWriteBuffer(){return(write_buffer);}

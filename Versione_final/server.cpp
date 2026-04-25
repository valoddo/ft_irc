/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cacorrea <cacorrea@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/13 14:04:12 by sel-khao          #+#    #+#             */
/*   Updated: 2026/04/25 17:45:27 by cacorrea         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.hpp"

void Server::sendReply(Client& client, const std::string& msg)
{
	client.getWriteBuffer() += msg;
	int fd = client.getClientFd();
	for (size_t i = 1; i < client_vect.size(); i++) {
		if (client_vect[i].getClientFd() == fd) {
			poll_fds[i].events |= POLLOUT;
			break;
		}
	}
}

Server::Server(const std::string& port, const std::string& password)
{
	server_name = SERVER_NAME;
	server_port = port;
	server_password = password;
	socket_fd = -1;
	initialized = false;
	int port_int = std::atoi(port.c_str());
	if (setupServer(port_int) == -1){
		std::cerr << "Failed to initialize server" << std::endl;
		return ;
	}
	struct pollfd server_pollfd;
	server_pollfd.fd = socket_fd;
	server_pollfd.events = POLLIN;
	server_pollfd.revents = 0; //aggiunto per valgrind
	poll_fds.push_back(server_pollfd);
	client_vect.push_back(Client());
	initialized = true;
}

Server::~Server(){
	if (socket_fd != -1)
		close(socket_fd);
	for (size_t i = 0; i < poll_fds.size(); i++)
		close(poll_fds[i].fd);
}

int Server::setupServer(int port)
{
	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_fd == -1){
		std::cout << "socket failed" << std::endl;
		return -1;
	}
	int opt = 1;
	setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	struct sockaddr_in address;
	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_port = htons(port);
	address.sin_addr.s_addr = INADDR_ANY;
	if (bind(socket_fd, (struct sockaddr*)&address, sizeof(address)) == -1){
		std::cout << "bind failed" << std::endl;
		return -1;
	}
	if (listen(socket_fd, 10) == -1){
		std::cout << "listen failed" << std::endl;
		return -1;
	}
	if (fcntl(socket_fd, F_SETFL, O_NONBLOCK) == -1){
		std::cout << "blocking fd didnt succeed" << std::endl;
		return -1;
	}
	return (socket_fd);
}

bool Server::isInitialized() const{return(initialized);}

const std::string& Server::getName() const{return(server_name);}

void Server::acceptNewClient()
{
	struct sockaddr_in client_addr;
	socklen_t addr_size = sizeof(client_addr);
	int client_socket = accept(socket_fd, (struct sockaddr*)&client_addr, &addr_size);
	if (client_socket == -1){
		std::cout << "accept failed" << std::endl;
		return;
	}
	if (fcntl(client_socket, F_SETFL, O_NONBLOCK) == -1){
		std::cout << "blocking fd didnt succeed" << std::endl;
		close(client_socket);
		return;
	}
	Client client;
	client.setClientFd(client_socket);
	char clientIP[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &client_addr.sin_addr, clientIP, INET_ADDRSTRLEN);
	client.setIP(std::string(clientIP));
	client_vect.push_back(client);
	
	struct pollfd client_pollfd;
	client_pollfd.fd = client_socket;
	client_pollfd.events = POLLIN;
	client_pollfd.revents = 0; //aggiunto per valgrind
	poll_fds.push_back(client_pollfd);
	std::cout << "Client " << client.getClientFd() << " connected: " << clientIP << " fd=" << client_socket << std::endl;
}

// Write data to client (send messages)
void Server::handleClientWrite()
{
	for (size_t i = 1; i < poll_fds.size(); i++)
	{
		if (poll_fds[i].revents & POLLOUT)
		{
			std::string& wbuf = client_vect[i].getWriteBuffer();
			if (!wbuf.empty())
			{
				ssize_t sent = send(poll_fds[i].fd, wbuf.c_str(), wbuf.size(), 0);
				if (sent > 0){
					wbuf.erase(0, sent);
				}
				else if (sent == -1){
					close(poll_fds[i].fd);
					poll_fds.erase(poll_fds.begin() + i);
					client_vect.erase(client_vect.begin() + i);
					i--;
					continue ;
				}
				if (wbuf.empty())
					poll_fds[i].events &= ~POLLOUT;
			}
		}
	}
}

// Reads data from client (receive messages)
bool Server::handleClientRead(size_t i)
{
	char buffer[512];
	ssize_t bytes_letti;
	bytes_letti = recv(poll_fds[i].fd, buffer, sizeof(buffer) - 1, 0);
	if (bytes_letti > 0)
	{
		if (i >= client_vect.size())
			return true;
		buffer[bytes_letti] = '\0';
		client_vect[i].getReadBuffer() += buffer;
		std::string localBuf = client_vect[i].getReadBuffer();
		client_vect[i].getReadBuffer().clear();
		while (true)
		{
			size_t pos = localBuf.find("\r\n");
			if (pos == std::string::npos)
				break;
			std::string command = localBuf.substr(0, pos);
			localBuf.erase(0, pos + 2);
			int fd = client_vect[i].getClientFd();
			processCommand(client_vect[i], command);
			if (i >= client_vect.size() || client_vect[i].getClientFd() != fd)
				return true;
			}
		client_vect[i].getReadBuffer() = localBuf;
	}
	else if (bytes_letti == 0) // client disconnesso
	{
		close(poll_fds[i].fd);
		poll_fds.erase(poll_fds.begin() + i);
		client_vect.erase(client_vect.begin() + i);
		i--;
		return true;
	}
	else // bytes_letti == -1
	{
		if (errno != EAGAIN && errno != EWOULDBLOCK)
		{
			close(poll_fds[i].fd);
			poll_fds.erase(poll_fds.begin() + i);
			client_vect.erase(client_vect.begin() + i);
			return true;
		}
	}
	return false;
}

void Server::processCommand(Client& client, const std::string& command)
{
	size_t spacePos = command.find(' ');
	std::string cmd = command.substr(0, spacePos);
	for (size_t i = 0; i < cmd.size(); i++)
		cmd[i] = toupper(cmd[i]);
	std::string params = "";
	if (spacePos != std::string::npos)
		params = command.substr(spacePos + 1);
	if (cmd == "CAP"){
		if (params.find("LS") == 0)
			sendReply(client, ":" + server_name + " CAP * LS :\r\n");
		return ;
	}
	if (cmd == "PASS")
	{
		execPass(client, params);
		return;
	}
	else if (cmd == "NICK")
	{
		execNick(client, params);
		return;
	}
	else if (cmd == "USER")
	{
		execUser(client, params);
		return;
	}
	if (!client.isAuthenticated())
	{
		sendReply(client, ":" + server_name + " 451 " + client.getNick() + " " + cmd + " :You have not registered\r\n");
		return;
	}
	if (cmd == "JOIN")
		execJoin(client, params);
	else if (cmd == "PRIVMSG")
		execPrivmsg(client, params);
	else if (cmd == "INVITE")
		execInvite(client, params);
	else if (cmd == "TOPIC")
		execTopic(client, params);
	else if (cmd == "MODE")
		execMode(client, params);
	else if (cmd == "KICK")
		execKick(client, params);
	else if (cmd == "QUIT")
		execQuit(client, params);
	else
		sendReply(client, ":" + server_name + " 421 " + client.getNick() + " " + cmd + " :Unknown command\r\n");
}

void Server::sendToClient(int client_fd, const std::string& message)
{
	for (size_t i = 1; i < client_vect.size() && i < poll_fds.size(); i++)
	{
		if (client_vect[i].getClientFd() == client_fd)
		{
			client_vect[i].getWriteBuffer() += message;
			poll_fds[i].events |= POLLOUT; 
			break;
		}
	}
}

void Server::run()
{
	if (!initialized || poll_fds.empty())
	{
		std::cerr << "Server is not initialized correctly" << std::endl;
		return;
	}
	std::cout << "Server running on port " << server_port << std::endl;
	for (;;)
	{
		if (poll(&poll_fds[0], poll_fds.size(), -1) == -1){
			std::cout << "poll failed" << std::endl;
			return;
		}
		size_t current_size = poll_fds.size();
		if (poll_fds[0].revents & POLLIN)
			acceptNewClient();
		for (size_t i = 1; i < current_size; i++)
		{
			if (i >= poll_fds.size()) break;
			if (poll_fds[i].revents & POLLIN)
			{
				if (handleClientRead(i))
					i--;
			}
		}
		for (size_t i = 1; i < poll_fds.size(); i++) {
			if (!client_vect[i].getWriteBuffer().empty())
				poll_fds[i].events |= POLLOUT;
			else
				poll_fds[i].events &= ~POLLOUT;
		}
		handleClientWrite();
	}
}

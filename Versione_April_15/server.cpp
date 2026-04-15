/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sel-khao <sel-khao@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/13 14:04:12 by sel-khao          #+#    #+#             */
/*   Updated: 2026/04/15 18:17:45 by sel-khao         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.hpp"

void handleClientWrite(std::vector <struct pollfd> poll_fds, std::vector <Client> client_vect)
{
	for (unsigned long i = 1; i < poll_fds.size(); i++) {
    	if (poll_fds[i].revents & POLLOUT) {
        	std::string& wbuf = client_vect[i].getWriteBuffer();
        	if (!wbuf.empty()) {
            	ssize_t sent = send(poll_fds[i].fd, wbuf.c_str(), wbuf.size(), 0);
            	if (sent > 0) {
                	wbuf.erase(0, sent);
            	}
            	if (wbuf.empty()) {
                	poll_fds[i].events &= ~POLLOUT;
            	}
        	}
    	}
	}
}

bool handleClientRead(unsigned long i, std::vector<struct pollfd>& poll_fds, std::vector<Client>& client_vect){
	char buffer[512];
	ssize_t bytes_letti;
	bytes_letti = recv(poll_fds[i].fd, buffer, 512, 0);
	if (bytes_letti > 0){
		buffer[bytes_letti] = '\0';
		std::cout << "ricevuto da client " << i << ": " << buffer << std::endl;
		client_vect[i].getBuffer() += buffer;
		std::string& buf = client_vect[i].getBuffer();
		size_t pos;
		while ((pos = buf.find("\r\n")) != std::string::npos){
			std::string command = buf.substr(0, pos);
			std::cout << "Comando completo: " << command << std::endl;
			buf.erase(0, pos + 2);
		}
	}
	else if (bytes_letti == 0){//client disconnesso
		close(poll_fds[i].fd);
		poll_fds.erase(poll_fds.begin() + i);
		client_vect.erase(client_vect.begin() + i);//da liberare porta
		i--;
		return true;//ho rimosso
	}
	else{
	if (errno != EAGAIN && errno != EWOULDBLOCK){
		close(poll_fds[i].fd);
		poll_fds.erase(poll_fds.begin() + i);
		client_vect.erase(client_vect.begin() + i);//da liberare porta
		return true;
		}
	}
    return false;//non ho rimosso nada
}

int acceptNewClient(int socket_fd, struct sockaddr_in &client_addr, std::vector <struct pollfd> &poll_fds, std::vector <Client> &client_vect){
	socklen_t addr_size = sizeof(client_addr);
	int client_socket = accept(socket_fd, (struct sockaddr*)&client_addr, &addr_size);
	if (client_socket == -1){
		std::cout << "accept failed" << std::endl;
		return -1;
	} 
	if (fcntl(client_socket, F_SETFL, O_NONBLOCK) == -1){
		std::cout << "blocking fd didnt succeed" << std::endl;
		return -1;
	}
	Client client;
	client.setFd(client_socket);
	char clientIP[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &client_addr.sin_addr, clientIP, INET_ADDRSTRLEN);
	client.setIP(std::string(clientIP));
	client_vect.push_back(client);
	struct pollfd client_pollfd;
	client_pollfd.fd = client_socket;
	client_pollfd.events = POLLIN;
	poll_fds.push_back(client_pollfd);
}

int setupServer(int port){
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1){
        std::cout << "socket failed" << std::endl;
        return -1;
    }
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

void sendToClient(int client_fd, const std::string& message, std::vector<Client>& clients, std::vector<struct pollfd>& poll_fds) {
    for (size_t i = 0; i < clients.size(); i++) {
		if (clients[i].getFd() == client_fd) {
			clients[i].getWriteBuffer() += message;
			poll_fds[i].events |= POLLOUT;
			break;
        }
    }
}
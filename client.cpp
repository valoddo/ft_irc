#include "client.hpp"

Client::Client(): client_fd(-1), authenticated(false){}

Client::~Client(){}

int Client::getFd() const{return(client_fd);}

const std::string& Client::getIp() const{return(client_ip);}

const std::string& Client::getNick() const{return(nickname);}

const std::string& Client::getUser() const{return(username);}

bool Client::isAuthenticated() const{return(authenticated);}

void Client::setFd(int fd){client_fd = fd;}

void Client::setIP(const std::string& ip){client_ip = ip;}

void Client::setNick(const std::string& nick){nickname = nick;}

void Client::setUser(const std::string& user){username = user;}

void Client::setAuthenticated(bool value){authenticated = value;}

std::string& Client::getBuffer(){return(buffer);}

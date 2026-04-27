/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ircbot.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cacorrea <cacorrea@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/23 13:17:42 by sel-khao          #+#    #+#             */
/*   Updated: 2026/04/27 16:23:35 by cacorrea         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ircbot.hpp"

IRCBot::IRCBot(const std::string& ip, int port, const std::string& pass, const std::string& nick, const std::string& chan) : sock_fd(-1), server_ip(ip), server_port(port), password(pass), nickname(nick), username(nick + "_bot"), channel(chan)
{
    if (channel[0] != '#')
        channel = "#" + channel;
    std::srand(std::time(NULL));
}

IRCBot::~IRCBot(void)
{
    if (sock_fd != -1) {
        sendRaw("QUIT :Bot is leaving, bye.");
        close(sock_fd);
    }
}

void IRCBot::sendRaw(const std::string& msg)
{
    std::string full = msg + "\r\n";
    size_t total_sent = 0;

    while (total_sent < full.size())
    {
        ssize_t sent = send(sock_fd, full.c_str() + total_sent, full.size() - total_sent, 0);
        if (sent <= 0)
            break;
        total_sent += sent;
    }
}

std::string IRCBot::receiveLine(void)
{
    size_t pos = read_buffer.find("\r\n");
    if (pos == std::string::npos)
    {
        char buffer[512];
        ssize_t bytes = recv(sock_fd, buffer, sizeof(buffer) - 1, 0);
        if (bytes <= 0)
            return "";
        buffer[bytes] = '\0';
        read_buffer += buffer;
        pos = read_buffer.find("\r\n");
        if (pos == std::string::npos)
            return "";
    }
    std::string line = read_buffer.substr(0, pos);
    read_buffer.erase(0, pos + 2);
    return line;
}

void IRCBot::registerToServer(void)
{
    if (!password.empty())
        sendRaw("PASS " + password);
    sendRaw("NICK " + nickname);
    sendRaw("USER " + username + " 0 * :" + username);
}

void IRCBot::joinChannel(void)
{
    sendRaw("JOIN " + channel);
    std::cout << "Entred the channel: " << channel << std::endl;
}

void IRCBot::sendHelp(const std::string& target)
{
    std::string help_msg = 
        "╔═════════════════════════════════════════════╗\n"
        "║           🤖 BOT AVAILABLE COMMANDS         ║\n"
        "╠═════════════════════════════════════════════╣\n"
        "║ !help    - Show these commands              ║\n"
        "║ !hello   - Greet the bot                    ║\n"
        "║ INVITE   - Invite a client to a channel     ║\n"
        "║ JOIN     - Join a specific channel          ║\n"
        "║ KICK     - Eject a client from the channel  ║\n"
        "║ TOPIC    - Change or view the channel topic ║\n"
        "║ MODE     - Change the channel’s mode        ║\n"        
        "║ QUIT     - Disconnect from server           ║\n" 
        "╚═════════════════════════════════════════════╝";
    
    sendRaw("PRIVMSG " + target + " :\n" + help_msg);
}

bool IRCBot::handleServerMessage(const std::string& line)
{
    if (line.compare(0, 4, "PING") == 0)
    {
        size_t colon = line.find(':');
        if (colon != std::string::npos)
            sendRaw("PONG :" + line.substr(colon + 1));
        else
            sendRaw("PONG");
        return true;
    }
    if (line.find(" ERROR ") != std::string::npos || line.compare(0, 5, "ERROR") == 0)
        return true;
    return false;
}

void IRCBot::processCommand(const std::string& sender, const std::string& target, const std::string& cmd, const std::string& args)
{
    //std::cout << "TEST PROCESSCOMMAND" << std::endl;
    (void)args;
    std::string reply_target = target;
    if (target == nickname)
        reply_target = sender;

    if (cmd == "!help") {
        sendHelp(reply_target);
    }
    else if (cmd == "!hello") {
        std::string response = "hello " + sender + " I'm happy to help you!";
        sendRaw("PRIVMSG " + reply_target + " :" + response);
        std::cout << "[BOT] Sent: PRIVMSG " << reply_target << " :" << response << std::endl;
    }
}

void IRCBot::parseMessage(const std::string& line)
{
    if (line.empty() || line[0] != ':')
        return;

    size_t prefix_end = line.find(' ');
    if (prefix_end == std::string::npos || prefix_end < 2)
        return;

    size_t exclam = line.find('!');
    if (exclam == std::string::npos || exclam > prefix_end)
        return;
    std::string sender = line.substr(1, exclam - 1);

    size_t cmd_start = prefix_end + 1;
    size_t cmd_end = line.find(' ', cmd_start);
    if (cmd_end == std::string::npos)
        return;
    std::string irc_cmd = line.substr(cmd_start, cmd_end - cmd_start);
    if (irc_cmd != "PRIVMSG")
        return;

    size_t target_start = cmd_end + 1;
    size_t target_end = line.find(' ', target_start);
    if (target_end == std::string::npos)
        return;
    std::string target = line.substr(target_start, target_end - target_start);

    size_t message_start = line.find(" :", target_end);
    if (message_start == std::string::npos)
        return;
    std::string message = line.substr(message_start + 2);
    if (message.empty() || message[0] != '!')
        return;

    size_t space = message.find(' ');
    std::string cmd = message.substr(0, space);
    std::string args = (space != std::string::npos) ? message.substr(space + 1) : "";

    std::cout << "[BOT] Command: " << cmd << " from " << sender
              << " in " << target << std::endl;
    processCommand(sender, target, cmd, args);
}



void IRCBot::connectToServer(void)
{
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        std::cerr << "couldn't create a socket" << std::endl;
        exit(1);
    }
    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(server_port);
    if (inet_pton(AF_INET, server_ip.c_str(), &addr.sin_addr) <= 0) {
        std::cerr << "IP address not valid " << std::endl;
        exit(1);
    }
    if (::connect(sock_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "failed connection on " << server_ip << ":" << server_port << std::endl;
        exit(1);
    }
    std::cout << "connected to " << server_ip << ":" << server_port << std::endl;
    registerToServer();
    std::cout << " Bot " << nickname << " is ready!" << std::endl;
}

void IRCBot::run(void)
{
    connectToServer();
    bool joined = false;
    while (true) {
        std::string line = receiveLine();
        if (line.empty())
            break;
        std::cout << "-> " << line << std::endl;
        if (handleServerMessage(line))
            continue;
        if (!joined && line.find(" 001 ") != std::string::npos) {
            joinChannel();
            joined = true;
        }
        parseMessage(line);
    }
    std::cerr << "Bot disconnected from server" << std::endl;
}

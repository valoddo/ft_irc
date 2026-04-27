/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ircbot.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vloddo <vloddo@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/23 13:17:42 by sel-khao          #+#    #+#             */
/*   Updated: 2026/04/24 18:00:34 by vloddo           ###   ########.fr       */
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
    send(sock_fd, full.c_str(), full.size(), 0);
    //std::cout << "-> " << msg << std::endl;
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
    sendRaw("USER " + username);
}

void IRCBot::joinChannel(void)
{
    sendRaw("JOIN " + channel);
    std::cout << "Entred the channel: " << channel << std::endl;
}

// void IRCBot::sendHelp()
// {
//     std::string msg = ": \n"
//     sendRaw("PRIVMSG " + sender + ":╔═════════════════════════════════════════════╗\n");
//     sendRaw("PRIVMSG " + sender + "║           🤖 BOT AVAILABLE COMMANDS         ║\n");
//     sendRaw("PRIVMSG " + sender + "╠═════════════════════════════════════════════╣\n");
//     sendRaw("PRIVMSG " + sender + "║ !help    - Mostra questi comandi            ║\n");
//     sendRaw("PRIVMSG " + sender + "║ KICK     - Eject a client from the channel  ║\n");
//     sendRaw("PRIVMSG " + sender + "║ INVITE   - Invite a client to a channel     ║\n");
//     sendRaw("PRIVMSG " + sender + "║ TOPIC   - Change or view the channel topic  ║\n");
//     sendRaw("PRIVMSG " + sender + "║ MODE   - Change the channel’s mode:         ║\n");
//     sendRaw("PRIVMSG " + sender + "╚═════════════════════════════════════════════╝\n");
// }

// void IRCBot::sendHelp()
// {
//     //std::cout << "TEST SENDHELP" << std::endl;
//     std::string msg =
//         ": \n"
//         "╔═════════════════════════════════════════════╗\n"
//         "║           🤖 BOT AVAILABLE COMMANDS         ║\n"
//         "╠═════════════════════════════════════════════╣\n"
//         "║ !help    - Mostra questi comandi            ║\n"
//         "║ KICK     - Eject a client from the channel  ║\n"
//         "║ INVITE   - Invite a client to a channel     ║\n"
//         "║ TOPIC    - Change or view the channel topic ║\n"
//         "║ MODE     - Change the channel’s mode        ║\n"
//         "╚═════════════════════════════════════════════╝\n";

//     send(sock_fd, msg.c_str(), msg.size(), 0);
// }


void IRCBot::sendHelp(const std::string& target)
{
    std::string help_msg = 
        "╔═════════════════════════════════════════════╗\n"
        "║           🤖 BOT AVAILABLE COMMANDS         ║\n"
        "╠═════════════════════════════════════════════╣\n"
        "║ !help    - Show these commands              ║\n"
        "║ !hello   - Greet the bot                    ║\n"
        "╚═════════════════════════════════════════════╝";
    
    sendRaw("PRIVMSG " + target + " :" + help_msg);
}

void IRCBot::processCommand(const std::string& sender, const std::string& target, const std::string& cmd, const std::string& args)
{
    //std::cout << "TEST PROCESSCOMMAND" << std::endl;
    (void)args;
    if (cmd == "!help") {
        sendHelp(target);
    }
    // else if (cmd == "!quote" || cmd == "!frase") {
    //     int idx = std::rand() % quotes.size();
    //     std::cout << target << "📢 " << quotes[idx] << std::endl;
    // }
    // else if (cmd == "!time") {
    //     time_t now = std::time(NULL);
    //     std::string time_str = std::ctime(&now);
    //     time_str.erase(time_str.length() - 1);
    //     std::cout << target << "🕐 " << time_str << std::endl;
    // }
    // else if (cmd == "!hello") {
    //     std::cout << target << "hello " << sender << "! i'm happy to see ya" << std::endl;
    // }
       else if (cmd == "!hello") {
        std::string response = "hello " + sender + "I'm happy to help you!";
        sendRaw("PRIVMSG " + target + " :" + response);
        std::cout << "[BOT] Sent: PRIVMSG " << target << " :" << response << std::endl;
    }
}

// void IRCBot::parseMessage(const std::string& line)
// {
//     size_t exclam = line.find('!');
//     sender = line.substr(1, exclam - 1);
//     size_t privmsg_pos = line.find("PRIVMSG");
//     size_t space1 = line.find(' ', privmsg_pos);
//     size_t space2 = line.find(' ', space1 + 1);
//     std::string target = line.substr(space1 + 1, space2 - space1 - 1);
//     size_t colon = line.find(':', space2);
//     if (colon == std::string::npos) {
//         std::cout << "  → no : found" << std::endl;
//         return;
//     }
//     std::string message = line.substr(colon + 1);
//     if (!message.empty() && message[0] == '!') {
//         size_t space = message.find(' ');
//         std::string cmd = message.substr(0, space);
//         std::string args = (space != std::string::npos) ? message.substr(space + 1) : "";
//         std::cout << "command: " << cmd << " da " << sender << std::endl;
//         processCommand(sender, target, cmd, args);
//     }
// }

void IRCBot::parseMessage(const std::string& line)
{
    // Cerca PRIVMSG
    size_t privmsg_pos = line.find("PRIVMSG");
    if (privmsg_pos == std::string::npos)
    {
        privmsg_pos = line.find("privmsg");
        if (privmsg_pos == std::string::npos)
            return;
    }
    
    // Estrai il sender (nickname)
    size_t exclam = line.find('!');
    if (exclam == std::string::npos || exclam == 0)
        return;
    std::string sender = line.substr(1, exclam - 1);
    
    // Estrai il target (canale o utente)
    size_t space1 = line.find(' ', privmsg_pos);
    if (space1 == std::string::npos)
        return;
    size_t space2 = line.find(' ', space1 + 1);
    if (space2 == std::string::npos)
        return;
    std::string target = line.substr(space1 + 1, space2 - space1 - 1);
    
    // Estrai il messaggio (dopo i due punti)
    size_t colon = line.find(':', space2);
    if (colon == std::string::npos)
        return;
    std::string message = line.substr(colon + 1);
    
    // Verifica se è un comando
    if (!message.empty() && message[0] == '!') {
        size_t space = message.find(' ');
        std::string cmd = message.substr(0, space);
        std::string args = (space != std::string::npos) ? message.substr(space + 1) : "";
        
        std::cout << "[BOT] Command: " << cmd << " from " << sender 
                  << " in " << target << std::endl;
        processCommand(sender, target, cmd, args);
    }
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
            continue;
        std::cout << "-> " << line << std::endl;
        if (!joined && line.find(" 001 ") != std::string::npos) {
            joinChannel();
            joined = true;
        }
        parseMessage(line);
    }
}
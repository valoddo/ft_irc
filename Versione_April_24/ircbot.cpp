/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ircbot.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vloddo <vloddo@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/23 13:17:42 by sel-khao          #+#    #+#             */
/*   Updated: 2026/04/23 20:08:22 by vloddo           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ircbot.hpp"

IRCBot::IRCBot(const std::string& ip, int port, const std::string& pass, const std::string& nick, const std::string& chan) : sock_fd(-1), server_ip(ip), server_port(port), password(pass), nickname(nick), username(nick + "_bot"), channel(chan)
{
    if (channel[0] != '#')
        channel = "#" + channel;
    setupQuotes();
    std::srand(std::time(NULL));
}

IRCBot::~IRCBot(void)
{
    if (sock_fd != -1) {
        sendRaw("QUIT :Bot is leaving, bye.");
        close(sock_fd);
    }
}

void IRCBot::setupQuotes(void)
{
    quotes.push_back(" Il successo è la somma di piccoli sforzi ripetuti giorno dopo giorno.");
    quotes.push_back(" Non aspettare. Il momento non sarà mai perfetto.");
    quotes.push_back(" Credi in te stesso e tutto sarà possibile.");
    quotes.push_back(" Ogni esperto è stato un principiante.");
    quotes.push_back(" La felicità non è qualcosa di già fatto. Viene dalle tue azioni.");
    quotes.push_back(" Studiare programmazione è come fare palestra per il cervello!");
    quotes.push_back(" Un bug al giorno toglie il sonno di torno... e poi lo risolvi!");
    quotes.push_back(" 42: L'unico modo per uscire dalla matrix è programmarla.");
    quotes.push_back("");
}

void IRCBot::sendRaw(const std::string& msg)
{
    std::string full = msg + "\r\n";
    send(sock_fd, full.c_str(), full.size(), 0);
    std::cout << "-> " << msg << std::endl;
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
    sendRaw("USER " + username + " Motivational Bot");
}

void IRCBot::joinChannel(void)
{
    sendRaw("JOIN " + channel);
    std::cout << "Entred the channel: " << channel << std::endl;
}

void IRCBot::sendHelp()
{
    std::string msg = " : \
    ╔══════════════════════════════════════╗\
    ║      🤖 BOT COMANDI DISPONIBILI      ║\
    ╠══════════════════════════════════════╣\
    ║ !help    - Mostra questi comandi     ║\
    ║ !quote   - Frase motivazionale       ║\
    ║ !time    - Orario attuale            ║\
    ║ !hello   - Saluta il bot             ║\
    ║ !frase   - Un'altra frase motivante  ║\
    ╚══════════════════════════════════════╝";
    sendRaw("PRIVMSG " + sender + msg);
}

void IRCBot::processCommand(const std::string& sender, const std::string& target, const std::string& cmd, const std::string& args)
{
    (void)args;
    if (cmd == "!help") {
        sendHelp();
    }
    else if (cmd == "!quote" || cmd == "!frase") {
        int idx = std::rand() % quotes.size();
        std::cout << target << "📢 " << quotes[idx] << std::endl;
    }
    else if (cmd == "!time") {
        time_t now = std::time(NULL);
        std::string time_str = std::ctime(&now);
        time_str.erase(time_str.length() - 1);
        std::cout << target << "🕐 " << time_str << std::endl;
    }
    else if (cmd == "!hello") {
        std::cout << target << "hello " << sender << "! i'm happy to see ya" << std::endl;
    }
}

void IRCBot::parseMessage(const std::string& line)
{
    //if (line.find("PRIVMSG") == std::string::npos) {
    //    std::cout << "  → not a PRIVMSG" << std::endl;
    //    return;
    //}
    size_t exclam = line.find('!');
    //if (exclam == std::string::npos) {
    //    std::cout << "  → no command for bot" << std::endl;
    //    return;
    //}
    sender = line.substr(1, exclam - 1);
    //std::cout << "  → Sender: " << sender << std::endl;
    //if (sender == nickname) {
    //    std::cout << "  → ignore my message" << std::endl;
    //    return;
    //}// Estrae il target (canale o nickname)
    size_t privmsg_pos = line.find("PRIVMSG");
    size_t space1 = line.find(' ', privmsg_pos);
    size_t space2 = line.find(' ', space1 + 1);
    std::string target = line.substr(space1 + 1, space2 - space1 - 1);
    //std::cout << ": " << std::endl;
    size_t colon = line.find(':', space2);
    if (colon == std::string::npos) {
        std::cout << "  → no : found" << std::endl;
        return;
    }
    std::string message = line.substr(colon + 1);
    //std::cout << ": " << message << std::endl;
    if (!message.empty() && message[0] == '!') {
        size_t space = message.find(' ');
        std::string cmd = message.substr(0, space);
        std::string args = (space != std::string::npos) ? message.substr(space + 1) : "";
        std::cout << "command: " << cmd << " da " << sender << std::endl;
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
        // Gestisci PING del server (altrimenti ti disconnette!)
        //if (line.substr(0, 4) == "PING") {
        //    std::string token = line.substr(5);
        //    sendRaw("PONG :" + token);
         //   continue;
        //}
        if (!joined && line.find(" 001 ") != std::string::npos) {
            joinChannel();
            joined = true;
        }
        parseMessage(line);
    }
}
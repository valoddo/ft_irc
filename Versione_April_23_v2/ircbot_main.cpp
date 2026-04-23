/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ircbot_main.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vloddo <vloddo@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/23 13:20:23 by sel-khao          #+#    #+#             */
/*   Updated: 2026/04/23 19:56:58 by vloddo           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ircbot.hpp"

int main(int argc, char** argv)
{
    std::string ip = "127.0.0.1";
    int port = 6667;
    std::string password = "";
    std::string nickname = "Bananito";
    std::string channel = "general";
    
    if (argc > 1) ip = argv[1];
    if (argc > 2) port = std::atoi(argv[2]);
    if (argc > 3) password = argv[3];
    if (argc > 4) nickname = argv[4];
    if (argc > 5) channel = argv[5];
    
    std::cout << "=== 🤖 IRC BOT v1.0 ===" << std::endl;
    std::cout << "📡 Server: " << ip << ":" << port << std::endl;
    std::cout << "👤 Nickname: " << nickname << std::endl;
    std::cout << "📢 Canale: " << channel << std::endl;
    std::cout << "======================================" << std::endl;
    
    IRCBot bot(ip, port, password, nickname, channel);
    bot.run();
    
    return 0;
}
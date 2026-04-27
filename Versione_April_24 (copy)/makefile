# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: cacorrea <cacorrea@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/04/24 10:24:41 by cacorrea          #+#    #+#              #
#    Updated: 2026/04/24 12:50:15 by cacorrea         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = ircserv
NAME_BOT = bananito

CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98

SRC = main.cpp \
		server.cpp \
		client.cpp \
		command.cpp \
		cmd_privmsg_kick.cpp \
		cmd_topic_mode.cpp \
		cmd_authenticate.cpp \
		channel.cpp

SRCBOT = ircbot.cpp \
		ircbot_main.cpp

all: $(NAME) $(NAME_BOT)

$(NAME): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(NAME) $(SRC)

$(NAME_BOT): $(SRCBOT)
	$(CXX) $(CXXFLAGS) -o $(NAME_BOT) $(SRCBOT)

clean:
	@echo "no object file to clean"

fclean: clean
	rm -f $(NAME) $(NAME_BOT)

re: fclean all

.PHONY: all clean fclean re
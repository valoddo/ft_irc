NAME = ircserv
NAME_BOT = bananito

CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98

SRC = main.cpp \
		server.cpp \
		client.cpp \
		command.cpp \
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
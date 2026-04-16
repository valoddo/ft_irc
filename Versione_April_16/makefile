NAME = ircserv

CXX = c++

CXXFLAGS = -Wall -Wextra -Werror -std=c++98

SRC = main.cpp \
		server.cpp \
		client.cpp \
		command.cpp

all: $(NAME)

$(NAME): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(NAME) $(SRC)

clean:
	@echo "no object file to clean"

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
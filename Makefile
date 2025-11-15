NAME = ircserv

CXX = c++
CXXFLAGS =  -Wall -Wextra -Werror -std=c++98 -Iincludes -g -D_GLIBCXX_DEBUG

SRC 	= main.cpp srcs/Client.cpp srcs/Server.cpp
OBJ 	= $(SRC:.cpp=.o)


all: $(NAME)
	@echo compiling...

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJ)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)
	@echo cleaning $(OBJ)...

fclean: clean
	rm -f $(NAME)
	@echo removing $(NAME)

re: fclean all

.PHONY: all clean fclean re

.SILENT:
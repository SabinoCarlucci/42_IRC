NAME = ircserv

CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -Iincludes -g -D_GLIBCXX_DEBUG#flag per debuggare

OBJ_DIR = objs

SRC = main.cpp \
      srcs/Client.cpp srcs/Server.cpp srcs/commands.cpp \
      srcs/Channel.cpp srcs/cmd_helpers.cpp

OBJ = $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(notdir $(SRC)))

all: $(NAME)

$(NAME): $(OBJ)
	@echo "Linking $(NAME)..."
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)


$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

vpath %.cpp srcs .

$(OBJ_DIR)/%.o: %.cpp | $(OBJ_DIR)
	@echo "Compiling $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@echo "Removing object files and directory $(OBJ_DIR)..."
	@rm -rf $(OBJ_DIR)

fclean: clean
	@rm -f $(NAME)
	@echo "Removing $(NAME)"

re: fclean all

.PHONY: all clean fclean re

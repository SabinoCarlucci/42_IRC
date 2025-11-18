NAME = ircserv

CXX = c++
CXXFLAGS =  -Wall -Wextra -Werror -std=c++98 -Iincludes -g -D_GLIBCXX_DEBUG #flag for debug in vscode

OBJ_DIR = objs

SRC 	= main.cpp srcs/Client.cpp srcs/Server.cpp srcs/commands.cpp srcs/Channel.cpp srcs/cmd_helpers.cpp
OBJ = $(addprefix $(OBJ_DIR)/, $(notdir $(SRC:.cpp=.o)))


all: $(NAME)
#	@echo compiling...

#$(NAME): $(OBJ)
#	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJ)

$(NAME): $(OBJ_DIR) $(OBJ)
	@echo "Linking $(NAME)..."
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)

$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

$(OBJ_DIR)/%.o: srcs/%.cpp
	@echo "Compiling $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: %.cpp
	@echo "Compiling $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

#%.o: %.cpp
#	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
#	rm -f $(OBJ)
#	@echo cleaning $(OBJ)...
	@echo "Removing object files and directory $(OBJ_DIR)..."
	@rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)
	@echo removing $(NAME)

re: fclean all

.PHONY: all clean fclean re $(OBJ_DIR)

# .SILENT:
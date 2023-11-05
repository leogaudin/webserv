# COLORS
GREEN = \033[0;32m
RED = \033[0;31m
RESET = \033[0m

CC = c++
CFLAGS = -Wall -Werror -Wextra -std=c++98

# INCLUDE: .h files or .a library folders
# SRC: .c files
SRC_DIR = src
OBJ_DIR = obj
INCLUDE_DIR = include

SRC_FILES = $(wildcard $(SRC_DIR)/*.cpp)
OBJ_FILES = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC_FILES))

NAME = webserv

.PHONY: all clean fclean re

all: $(NAME)

$(NAME): $(OBJ_FILES)
	@$(CC) $(CFLAGS) -o $@ $^
	@echo "$(GREEN)+ $(NAME)$(RESET)"


$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	@$(CC) $(CFLAGS) -I$(INCLUDE_DIR) -c -o $@ $<

clean:
	@$(RM) -r $(OBJ_DIR)

fclean: clean
	@$(RM) $(NAME)
	@echo "$(RED)- $(NAME)$(RESET)"

re: fclean all

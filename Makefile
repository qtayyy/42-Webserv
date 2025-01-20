#------Colours------#
DEF_COLOR='\033[0;39m'
RESET = '\033[0;0m'
BLACK='\033[0;30m'
RED='\033[1;91m'
GREEN='\033[1;92m'
YELLOW='\033[0;93m'
BLUE='\033[0;94m'
MAGENTA='\033[0;95m'
CYAN='\033[0;96m'
GRAY='\033[0;90m'
WHITE='\033[0;97m'

NAME	= webserv

CC		= c++
CFLAGS	= -Werror -Wextra -Wall -std=c++98 #-g3 -fsanitize=address

#---------Sources--------#
SRC_PATH = ./srcs/
SRC		= 	Block.cpp \
			Client.cpp \
			Cluster.cpp \
			LocationBlock.cpp \
			main.cpp \
			ServerBlock.cpp \
			utils.cpp

SRCS	= $(addprefix $(SRC_PATH), $(SRC))

#---------Objects--------#
OBJ_PATH	= ./objects/
OBJ			= $(SRC:.cpp=.o)
OBJS		= $(addprefix $(OBJ_PATH), $(OBJ))

#--------Includes--------#
INC			=	-I ./includes/


#--------Main rule---------#
all: $(OBJ_PATH) $(NAME)

#--------Objects directory rule---------#
$(OBJ_PATH):
	@ mkdir -p $(OBJ_PATH)

#--------Objects rule---------#
$(OBJ_PATH)%.o: $(SRC_PATH)%.cpp
	@ $(CC) $(CFLAGS) -c $< -o $@ $(INC)

#--------Project file rule---------#
$(NAME): $(OBJS)
	@ echo $(BLUE)"Compiling Webserv..."$(RESET)
	@ $(CC) $(CFLAGS)  $(OBJS) -o $@ $(INC)
	@ echo $(GREEN)"Webserv compiled"$(RESET)
	@ echo $(MAGENTA)"Your Program Name :"$(RESET)$(YELLOW)" $(NAME) "$(RESET)

clean:
	@ echo $(CYAN)"Cleaning Webserv objects..."$(RESET)
	@ rm -rf $(OBJ_PATH)
	@ echo $(GREEN)"Objects cleaned"$(RESET)

fclean: clean
	@ echo $(RED)"Cleaning everything..."$(RESET)
	@ rm -f $(NAME)
	@ echo $(GREEN)"Everything cleaned"$(RESET)

re: fclean all

.PHONY: all re clean fclean

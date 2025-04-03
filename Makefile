##
## EPITECH PROJECT, 2025
## Makefile
## File description:
## Makefile
##

NAME = my_radar

SRC = src/main.c \
      src/parser.c \
	  src/simulation.c \
	  src/update_render.c \

OBJ = $(SRC:.c=.o)

CFLAGS = -I./include

LDFLAGS = -lcsfml-graphics -lcsfml-window -lcsfml-system -lcsfml-audio -lm

all: $(NAME)

$(NAME): $(OBJ) $(LIB)
	gcc -o $(NAME) $(OBJ) $(CFLAGS) $(LDFLAGS)

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all

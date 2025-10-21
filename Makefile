SRC= main.c ft_irc.cpp client.cpp utils.cpp

OBJ =${SRC:.c=.o}

CFLAGS = -Wall -Werror -Wextra -g -std=c++98

CC = c++

RM = rm -f

NAME = irc

.c.o:
	@${CC} ${CFLAGS} -c $< -o ${<:.c=.o}

all : ${NAME}

${NAME} : ${OBJ}
	@${CC} ${CFLAGS} ${OBJ} -o ${NAME}

clean :
	@${RM} *.o

fclean : clean
	@${RM} ${NAME}

re : fclean all clean

.PHONY: all clean fclean re

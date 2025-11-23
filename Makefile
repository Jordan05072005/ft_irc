SRC = srcs/main.cpp \
		srcs/Bot.cpp \
		srcs/Channel.cpp \
		srcs/Client.cpp \
		srcs/Server.cpp \
		srcs/ServerCmd.cpp \
		srcs/ServerMess.cpp \
		srcs/ServerUtils.cpp \
		srcs/signal.cpp \
		srcs/utils.cpp

OBJ = ${SRC:.c=.o}

CFLAGS = -Wall -Werror -Wextra -g -std=c++98

CC = c++

RM = rm -f

NAME = ircserv

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

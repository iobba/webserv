PREFIX			= ./srcs/

SRCS			= client.cpp main.cpp request.cpp server.cpp ServManager.cpp webserv.cpp parseconfig.cpp Location.cpp

SRCS_ALL		= $(addprefix $(PREFIX), $(SRCS))

NAME			= webserv

CPPFLAGS		= -Wall -Wextra -Werror -std=c++98

RM				= rm -f

AR				= ar rc 

OBJS			= $(SRCS_ALL:.cpp=.o)

BODY_FILES		= /tmp/*.txt

all : $(NAME)

$(NAME) : $(OBJS)
	g++ ${OBJS} -o ${NAME}

clean :
	$(RM) $(OBJS) $(BODY_FILES)

fclean : clean
	$(RM) $(NAME)

re : fclean all
NAME = mini_irc

SRC = main.cpp Server.cpp Client.cpp Chanel.cpp

FLAGS = -Wall -Wextra -Werror

OBJ = $(SRC:.cpp=.o)

all : $(NAME)

$(NAME) :
	g++ $(FLAGS) $(SRC) -o $(NAME)
	rm -f $(OBJ)

clean :
	rm -f $(OBJ)

fclean :
	rm -f $(NAME)

re : fclean all
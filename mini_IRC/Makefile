NAME = IRC

CPP			 = c++
CPPFLAGS	 = -std=c++98
CPPFLAGS	+= -Wall -Wextra -Werror -Wshadow -pedantic
CPPFLAGS	+= -g

SRC = $(wildcard *.cpp)

OBJDIR		= .obj
OBJ = $(patsubst %.cpp, $(OBJDIR)/%.o, $(SRC))

all: $(NAME)


run: $(NAME)
	@./$(NAME)

# Directories
$(NAME): $(OBJ)
	$(CPP) -o "$@" $(CPPFLAGS) $(OBJ)

# Common rules
$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	@$(CPP) -o "$@" $(CPPFLAGS) "$<" -c

clean:
	$(RM) $(OBJ)
	-@rmdir $(OBJDIR) 2>/dev/null || true

fclean: clean
	$(RM) $(NAME)

re: fclean
	$(MAKE) $(NAME)

# val: 
# 	valgrind --leak-check=full ./$(NAME)

.PHONY: all clean fclean re val
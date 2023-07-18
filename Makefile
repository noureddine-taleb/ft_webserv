SRCS := http.cpp socket.cpp lib.cpp main.cpp watchlist.cpp config.cpp sched.cpp form-data.cpp	\
		response/response.cpp response/lib2.cpp response/Response_error.cpp response/read_file.cpp 	\
		response/post.cpp response/delete.cpp response/entry.cpp response/generate_res.cpp response/read_dir.cpp\
		response/redirection.cpp response/cgi.cpp response/get.cpp response/create_file.cpp response/path.cpp\

NAME = webserv

CPP = c++

CPPFLAGS = -Wall -Wextra -Werror -Iincludes -std=c++98 

OBJS = $(SRCS:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CPP) $(CPPFLAGS) $(OBJS) -o $(NAME)
	
clean :
	@rm -f *.o **/*.o
	
fclean : clean
	@rm -f $(NAME)

re : fclean all

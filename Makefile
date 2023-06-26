CXX := c++
CXXFLAGS := -Wall -Wextra -g -Iincludes -std=c++98
TARGET := webserv
SRCS := http.cpp socket.cpp lib.cpp main.cpp watchlist.cpp config.cpp http.cpp sched.cpp form-data.cpp	\
		response/response.cpp response/lib2.cpp response/Response_error.cpp response/read_file.cpp 	\
		response/post.cpp response/delete.cpp response/entry.cpp response/generate_res.cpp response/read_dir.cpp
OBJS := ${SRCS:.cpp=.o}

all: $(TARGET)

$(TARGET): $(OBJS)
	@$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	-rm -f **/*.o

fclean: clean
	-rm -f $(TARGET)

re: fclean all

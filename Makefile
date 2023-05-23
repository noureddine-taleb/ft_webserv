CXX := g++
CXXFLAGS := -Wall -Wextra #-std=c++98
TARGET := webserv
SRCS := http.cpp lib.cpp main.cpp epoll.cpp config.cpp

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:

fclean: clean
	-rm -f $(TARGET)

re: fclean all

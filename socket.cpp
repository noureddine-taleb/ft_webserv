#include "webserv.hpp"
#include <set>
#ifdef __APPLE__
#include <sys/event.h>
#elif __linux__
#include <sys/epoll.h>
#endif
#include <fcntl.h>
#include <sstream>
#include <netdb.h>

/**
 * spawn servers and add their sockets to watchlist
 * wait for events (connections, requests) and handle them serially
 */
void spawn_servers(int wfd) {
  std::set<std::string> servers;
  for (size_t i = 0; i < config.servers.size(); i++) {
	std::stringstream gstream;
	gstream << config.servers[i].port;
	std::string port = gstream.str();
	servers.insert(config.servers[i].ip + ":" + port);
  }
  for (std::set<std::string>::iterator it = servers.begin();
	   it != servers.end(); it++) {
	std::vector<std::string> v = split(*it, ":");
	std::string ip = v[0];
	std::string port = v[1];

	struct addrinfo hints, *res;
	int error;

	std::memset(&hints, 0, sizeof(hints));
	hints.ai_family = PF_INET;
	hints.ai_socktype = SOCK_STREAM;
	error = getaddrinfo(ip.c_str(), port.c_str(), &hints, &res);
	if (error)
		die(gai_strerror(error));

	int sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	assert_msg(sock != -1, "socket: " << strerror(errno));
	freeaddrinfo(res);

	int enable = 1;
	assert_msg(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) == 0,
		   "setsockopt: " << strerror(errno));

	assert_msg(bind(sock, res->ai_addr, res->ai_addrlen) == 0,
		   "bind() failed");

	assert_msg(listen(sock, BACKLOG_SIZE) == 0, "listen: " << strerror(errno));
	debug("listening on: " << ip << ":" << port);
#ifdef __APPLE__
	watchlist_add_fd(wfd, sock, EVFILT_READ);
#elif __linux__
	watchlist_add_fd(wfd, sock, EPOLLIN);
#endif
	config.max_server_fd = sock;
  }
}

void accept_connection(int wfd, int server) {
  struct sockaddr_in caddress;
  socklen_t len = sizeof caddress;
  int client = accept(server, (struct sockaddr *)&caddress, &len);
  assert_msg(client != -1, "accept: " << strerror(errno));
  assert_msg (fcntl(client, F_SETFL, O_NONBLOCK) >= 0, "fcntl: " << strerror(errno));
#ifdef __APPLE__
  watchlist_add_fd(wfd, client, EVFILT_READ);
#elif __linux__
  watchlist_add_fd(wfd, client, EPOLLIN);
#endif

  debug("connection received " << inet_ntoa(caddress.sin_addr) << ":"
							   << ntohs(caddress.sin_port));
}

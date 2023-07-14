#include "webserv.hpp"
#include <set>
#ifdef __APPLE__
#include <sys/event.h>
#elif __linux__
#include <sys/epoll.h>
#endif
#include <fcntl.h>
#include <sstream>

// todo: move these functions into a separate files
// todo: connection: keep-alive
/**
 * spawn servers and add their sockets to watchlist
 * wait for events (connections, requests) and handle them serially
 * todo: multiple servers could use the same port
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
	int sock = socket(PF_INET, SOCK_STREAM, 0);
	assert_msg(sock != -1, "socket() failed");

	int enable = 1;
	assert_msg(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable,
					  sizeof(enable)) == 0,
		   "setsockopt() failed");

	std::vector<std::string> v = split(*it, ":");
	std::string ip = v[0];
	int port = ft_stoi(v[1]);
	// todo: use getaddrinfo
	struct sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr(ip.c_str());
	address.sin_port = htons(port);

	assert_msg(bind(sock, (struct sockaddr *)&address, sizeof(address)) == 0,
		   "bind() failed");

	assert_msg(listen(sock, BACKLOG_SIZE) == 0, "listen() failed");
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
  assert_msg(client != -1, "accept() failed");
  assert_msg (fcntl(client, F_SETFL, O_NONBLOCK) >= 0, "fcntl() failed");
#ifdef __APPLE__
  watchlist_add_fd(wfd, client, EVFILT_READ);
#elif __linux__
  watchlist_add_fd(wfd, client, EPOLLIN);
#endif

  debug("connection received " << inet_ntoa(caddress.sin_addr) << ":"
							   << ntohs(caddress.sin_port));
}

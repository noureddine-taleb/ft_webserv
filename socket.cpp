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
	std::vector<struct addrinfo *> servers;
	static const int enable = 1;
	int sock;

  for (std::vector<Server>::iterator it = config.servers.begin(); it < config.servers.end(); it++) {
	struct addrinfo hints, *res;
	int error;

	std::memset(&hints, 0, sizeof(hints));
	hints.ai_family = PF_INET;
	hints.ai_socktype = SOCK_STREAM;
	error = getaddrinfo(it->config_ip.c_str(), it->config_port.c_str(), &hints, &res);
	if (error)
		die(gai_strerror(error));

	struct sockaddr_in *cur = (struct sockaddr_in *)res->ai_addr;
	it->__ip = cur->sin_addr.s_addr;
	it->__port = cur->sin_port;
	for (std::vector<struct addrinfo *>::iterator it2 = servers.begin(); it2 < servers.end(); it2++) {
		struct sockaddr_in *target = (struct sockaddr_in *)(*it2)->ai_addr;
		if (target->sin_addr.s_addr == it->__ip && target->sin_port == it->__port) {
			debug("skip = server=" << it->__ip << ":" << ntohs(it->__port) << " same as server=" << target->sin_addr.s_addr << ":" << ntohs(target->sin_port));
			freeaddrinfo(res);
			goto skip;
		}
	}
	sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	assert_msg(sock != -1, "socket: " << strerror(errno));
	assert_msg(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) == 0,
		   "setsockopt: " << strerror(errno));

	assert_msg(bind(sock, res->ai_addr, res->ai_addrlen) == 0,
			"two identical servers bind(" << it->config_ip.c_str() << ":" << it->config_port.c_str() << ") failed = " << strerror(errno));

	assert_msg(listen(sock, BACKLOG_SIZE) == 0, "listen: " << strerror(errno));
	debug("listening on: " << it->config_ip << ":" << it->config_port);
#ifdef __APPLE__
	watchlist_add_fd(wfd, sock, EVFILT_READ);
#elif __linux__
	watchlist_add_fd(wfd, sock, EPOLLIN);
#endif
	it->__fd = sock;
	config.max_server_fd = sock;
	servers.push_back(res);
skip:
	continue;
  }
	// cleanup
	for (std::vector<struct addrinfo *>::iterator it = servers.begin(); it < servers.end(); it++) {
		freeaddrinfo(*it);
	}
}

int accept_connection(int wfd, int server) {
  struct sockaddr_in caddress;
  socklen_t len = sizeof caddress;
  int client = accept(server, (struct sockaddr *)&caddress, &len);
  assert_msg(client != -1, "accept: " << strerror(errno));
  assert_msg (fcntl(client, F_SETFL, O_NONBLOCK) >= 0, "fcntl: " << strerror(errno));
  watchlist_insert(wfd, client);

  debug(GREEN << "connection received " << inet_ntoa(caddress.sin_addr) << ":" << ntohs(caddress.sin_port) << END);
  return client;
}

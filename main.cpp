#ifdef __APPLE__
#include <sys/event.h>
#elif __linux__
#include <sys/epoll.h>
#endif
#include "webserv.hpp"

#define debug(msg) std::cerr << msg << __FILE__ << ":" << __LINE__;

void die(std::string msg) {
	perror(msg.c_str());
	exit(1);
}

// todo: check if globals are allowed
Config config;
int max_server_fd;
/**
 * spawn servers and add their sockets to watchlist
 * wait for events (connections, requests) and handle them serially
 * 
*/

void spawn_servers(int wfd) {
	for (size_t i = 0; i < config.servers.size(); i++) {
		int sock;
		assert((sock = socket(PF_INET, SOCK_STREAM, 0)) != -1);

		int enable = 1;
		assert(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) == 0);

		struct sockaddr_in address;
		address.sin_family = AF_INET;
		address.sin_addr.s_addr = inet_addr(config.servers[i].ip.c_str());
		address.sin_port = htons(config.servers[i].port);

		assert(bind(sock, (struct sockaddr *)&address, sizeof(address)) == 0);

		assert(listen(sock, BACKLOG_SIZE) == 0);
		std::cout << "--------- " << "listening on: " << config.servers[i].ip << ":" << config.servers[i].port << std::endl;

#ifdef __APPLE__
		watchlist_add_fd(wfd, sock, EVFILT_READ);
#elif __linux__
		watchlist_add_fd(wfd, sock, EPOLLIN);
#endif
		max_server_fd = sock;
	}
}

void accept_connection(int wfd, int server) {
	struct sockaddr_in caddress;
	socklen_t len = sizeof caddress;
	int client;
	assert((client = accept(server, (struct sockaddr *)&caddress, &len)) != -1);
	
#ifdef __APPLE__
		watchlist_add_fd(wfd, client, EVFILT_READ);
#elif __linux__
		watchlist_add_fd(wfd, client, EPOLLIN);
#endif

	std::cout << "--------- " << "connection received " << inet_ntoa(caddress.sin_addr) << ":" << ntohs(caddress.sin_port) << std::endl;
}

/**
 * get and parse http request from fd
 * @return:
 * 0: success
 * -1: connection is broken and should be closed
 * -x: http status code
*/
int get_request(int fd, HttpRequest &request) {
	int ret;
	char buffer[255];
	std::string	http_rem;
	bool done;

	while (1) {
		done = false;
		if ((ret = recv(fd, buffer, sizeof(buffer) - 1, 0)) < 0)
			return -1;
		if (ret == 0) {
			debug("recv == 0\n");
			return -1;
		}
		buffer[ret] = 0;
		http_rem += buffer;
		int parsed = parse_partial_http_request(http_rem, request, &done);
		if (parsed < 0)
			return parsed;
		http_rem.erase(0, parsed);
		if (done)
			break;
	}
	if (http_rem.length())
		debug("http_rem still contains data\n");
	return 0;
}

int check_request_handler(HttpRequest &request) {
	std::vector<Server>::iterator server_it;
	std::vector<Location>::iterator location_it;

	server_it = server(config, request);
	location_it = location(config, request, server_it);

	if (location_it == server_it->routes.end())
		return -404;

	if (std::find(location_it->methods.begin(), location_it->methods.end(), request.method) == location_it->methods.end())
		return -405;

	return 0;
}
////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv) {
	if (argc != 2)
		die("usage: webserv <config_file>\n");

	parse_config(argv[1]);
	// dump_config(config);

    int wfd = init_watchlist();
	spawn_servers(wfd);

	int ret;

	std::map<int, SchedulableEntity>	clients;
	while (1) {
		int fd = watchlist_wait_fd(wfd);

		assert(fd >= 0);
		if (fd <= max_server_fd) {
			accept_connection(wfd, fd);
			continue;
		}

		int							status_code;
		HttpRequest					request;
		HttpResponse				response;
		std::string					response_buffer;
		std::string					content_length;
		std::map<int, HttpResponse>::iterator clients_it = clients.find(fd);
		
		status_code = get_request(fd, request);

		if (status_code < 0) {
			if (status_code == -1)
				goto close_socket;
			goto error_response;
		}

		std::cout << "--------- handleable request received"<< std::endl;

		// request parsed
		// todo: check client_max_body
process:
		int new_request(HttpRequest &request, &response, status_code);
		int continue_previous_response(HttpResponse &response);
close_socket:
			std::cout << "--------- invalid request: close socket"<< std::endl;
			clients.erase(fd);
			watchlist_del_fd(wfd, fd);
			close(fd);
	}
}

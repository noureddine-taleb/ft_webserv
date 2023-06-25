#ifdef __APPLE__
#include <sys/event.h>
#elif __linux__
#include <sys/epoll.h>
#endif
#include "webserv.hpp"
#include <cstring>
#include <set>
#include "sched.hpp"

void die(std::string msg) {
	perror(msg.c_str());
	exit(1);
}

// todo: check if globals are allowed
Config config;
int max_server_fd;
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
		servers.insert(config.servers[i].ip + ":" + std::to_string(config.servers[i].port));
	}
	for (std::set<std::string>::iterator it = servers.begin(); it != servers.end(); it++) {
		int sock;
		assert((sock = socket(PF_INET, SOCK_STREAM, 0)) != -1);

		int enable = 1;
		assert(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) == 0);

		std::vector<std::string> v = split(*it, ":");
		std::string ip = v[0];
		int port = std::stoi(v[1]);
		struct sockaddr_in address;
		address.sin_family = AF_INET;
		address.sin_addr.s_addr = inet_addr(ip.c_str());
		address.sin_port = htons(port);

		assert(bind(sock, (struct sockaddr *)&address, sizeof(address)) == 0);

		assert(listen(sock, BACKLOG_SIZE) == 0);
		std::cout << "--------- " << "listening on: " << ip << ":" << port << std::endl;

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
 * -2: request not finished yet: to be continued
 * x: http failure status code (4xx, 5xx)
*/
int get_request(int fd, HttpRequest &request) {
	int ret;
	char buffer[255];
	bool done;
	int iter = 0;
	int max_iter = 5;

	while (1) {
		done = false;
		if ((ret = recv(fd, buffer, sizeof(buffer) - 1, 0)) < 0)
			return REQ_CONN_BROKEN;
		if (ret == 0) {
			debug("recv == 0\n");
			return REQ_CONN_BROKEN;
		}
		int last_size = request.http_rem.size();
		request.http_rem.resize(last_size + ret);
		memcpy(&request.http_rem[last_size], buffer, ret);
		int ret = parse_partial_http_request(request.http_rem, request, &done);
		if (ret < 0)
			return -ret;
		if (done)
			break;
		iter++;
		if (iter >= max_iter)
			return REQ_TO_BE_CONT;
	}
	if (request.http_rem.size())
		debug("http_rem still contains data\n");
	return 0;
}

int main(int argc, char **argv) {
	if (argc != 2)
		die("usage: webserv <config_file>\n");

	parse_config(argv[1]);
	// dump_config(config);

    int wfd = init_watchlist();
	spawn_servers(wfd);

	int									finished;
	std::map<int, SchedulableEntity *>	tasks;

	while (1) {
		int							status_code = 0;
		HttpRequest					request;
		HttpResponse				response;

		int fd = watchlist_wait_fd(wfd);

		if (fd > 2 && fd <= max_server_fd) {
			accept_connection(wfd, fd);
			continue;
		}
		// no new request, serve pending ones
		if (fd == WATCHL_NO_PENDING) {
			fd = sched_get_starved(tasks);
			if (fd == Q_EMPTY)
				continue;

			if (tasks[fd]->get_type() == REQUEST) {
				request = *dynamic_cast<HttpRequest *>(tasks[fd]); 
				goto request;
			} else if (tasks[fd]->get_type() == RESPONSE) {
				response = *dynamic_cast<HttpResponse *>(tasks[fd]);
				goto response;
			}
		}

request:
		status_code = get_request(fd, request);
		switch (status_code)
		{
		case REQ_CONN_BROKEN:
			goto close_socket;
			break;
		case REQ_TO_BE_CONT:
			sched_queue_task(tasks, fd, new HttpRequest(request));
			continue;
		default:
			sched_unqueue_task(tasks, fd);
			break;
		}
response:
		finished = send_response(fd, request, response, status_code);
		if (finished) {
			sched_unqueue_task(tasks, fd);
			goto close_socket;
		} else {
			sched_queue_task(tasks, fd, new HttpResponse(response));
		}

		continue;
close_socket:
		std::cout << "--------- invalid request: close socket"<< std::endl;
		sched_unqueue_task(tasks, fd);
		watchlist_del_fd(wfd, fd);
		close(fd);
	}
}

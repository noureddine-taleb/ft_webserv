#ifdef __APPLE__
#include <sys/event.h>
#elif __linux__
#include <sys/epoll.h>
#endif
#include "webserv.hpp"
#include <cstring>
#include "sched.hpp"

// todo: check if globals are allowed
Config config;

int main(int argc, char **argv) {
	if (argc != 2)
		die("usage: webserv <config_file>\n");

	parse_config(argv[1]);
	// dump_config(config);

    int wfd = init_watchlist();
	spawn_servers(wfd);

	int									finished = 0;
	std::map<int, SchedulableEntity *>	tasks;

	while (1) {
		int							status_code = 0;
		HttpRequest					request;
		HttpResponse				response;

		int fd = watchlist_wait_fd(wfd);

		if (fd > 2 && fd <= config.max_server_fd) {
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
		// finished = send_response(fd, request, response, status_code);
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

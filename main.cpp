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
	bool 								close_connexion = false;

	while (1) {
		int							status_code = 0;
		HttpRequest					request;
		HttpResponse				response;
		HttpRequest					*copy_request;

		int fd = watchlist_wait_fd(wfd);

		if (fd > 2 && fd <= config.max_server_fd) {
			accept_connection(wfd, fd);
			continue;
		}
		// no new request, serve pending ones
		if (fd == WATCHL_NO_PENDING || tasks.find(fd) != tasks.end()) {
			fd = sched_get_starved(tasks);
			if (fd == Q_EMPTY)
				continue;
			std::cout << "schedule pending requests" << std::endl;
			if (tasks[fd]->get_type() == REQUEST) {
				copy_request = dynamic_cast<HttpRequest *>(tasks[fd]);
				request = *copy_request; 
				goto request;
			} else if (tasks[fd]->get_type() == RESPONSE) {
				response = *dynamic_cast<HttpResponse *>(tasks[fd]);
				goto response;
			}
		}
request:
		status_code = get_request(fd, request);
		std::cout << "status_code = " << status_code << std::endl;
		switch (status_code)
		{
		case REQ_CONN_BROKEN:
			goto close_socket;
			break;
		case REQ_TO_BE_CONT:
			copy_request = new HttpRequest(request);
			sched_queue_task(tasks, fd, copy_request);
			continue;
		default:
			sched_unqueue_task(tasks, fd);
			break;
		}
response:
		// dump_request(request);
		// goto close_socket;
		// std::cout << "\033[32m"  << "method: " << request.method<< "\033[0m" << std::endl;
		// std::cout << "\033[32m"  << "url: " << request.url<< "\033[0m" << std::endl;
		// std::cout << "\033[32m"  << "version: " << request.version << "\033[0m" << std::endl;
		// for (std::map<std::string , std::string>::iterator it = request.headers.begin(); it != request.headers.end(); it++) {
		// 	std::cout << "\033[32m" << it->first << ' ' << it->second << "\033[0m" << std::endl;
		// }
		finished = send_response(fd, request, response, status_code, &close_connexion);
		
		// std::cout << "*********************>finished = "<< finished << std::endl;
///////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
		// finished = send_response(fd, request, response, status_code);
		// finished = send_response(fd, request, response, status_code, &close);
		// std::cout << close_connexion << "= " << std::endl;
		if (finished || close_connexion) {
			sched_unqueue_task(tasks, fd);
			goto close_socket;
		} else {
			sched_queue_task(tasks, fd, new HttpResponse(response));
		}
		if (close_connexion)
			goto close_socket;

		continue;
close_socket:
		std::cout << "--------- closing socket"<< std::endl;
		sched_unqueue_task(tasks, fd);
		watchlist_del_fd(wfd, fd);
		close(fd);
	}
}

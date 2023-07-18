#ifdef __APPLE__
#include <sys/event.h>
#elif __linux__
#include <sys/epoll.h>
#endif
#include "sched.hpp"
#include "webserv.hpp"
#include <cstring>
#include <signal.h>

// todo: check if globals are allowed
Config config;

void ignore(int sig)
{
	(void)sig;
}

Server get_server(int fd)
{
	for (std::vector<Server>::iterator it = config.servers.begin(); it != config.servers.end(); it++)
	{
		if (it->__fd == fd)
		{
			return *it;
		}
	}
	die("unreachable: source server for a request cannot be found fd = " << fd);
	Server *dummy = new Server();
	return *dummy;
}

int main(int argc, char **argv)
{
	if (argc != 2)
		die("usage: webserv <config_file>\n");

	signal(SIGPIPE, SIG_IGN);
	parse_config(argv[1]);
	dump_config(config);

	int wfd = init_watchlist();
	spawn_servers(wfd);

	int finished = 0;
	std::map<int, SchedulableEntity *> tasks;
	std::map<int, Server> connexion_srcs;
	bool close_connexion = false;

	while (1)
	{
		int status_code = 0;
		HttpRequest request;
		HttpResponse response;
		HttpRequest *copy_request;

		// debug("entering watchlist");
		int fd = watchlist_wait_fd(wfd);

		// debug("watchlist returned = " << fd);
		if (fd > 2 && fd <= config.max_server_fd)
		{
			int conn = accept_connection(wfd, fd);
			connexion_srcs[conn] = get_server(fd);
			continue;
		}
		// no new request, serve pending ones
		if (tasks.find(fd) != tasks.end())
		{
			if (tasks[fd]->get_type() == REQUEST)
			{
				// debug("schedule pending requests");
				copy_request = dynamic_cast<HttpRequest *>(tasks[fd]);
				request = *copy_request;
				if (request.finished)
					goto response;
				goto request;
			}
			else if (tasks[fd]->get_type() == RESPONSE)
			{
				// debug("schedule pending response");
				response = *dynamic_cast<HttpResponse *>(tasks[fd]);
				goto response;
			}
		}
		else
		{
			// debug("received new request");
			request.ip = connexion_srcs[fd].__ip;
			request.port = connexion_srcs[fd].__port;
		}

	request:
		status_code = get_request(fd, request);
		switch (status_code)
		{
		case REQ_CONN_BROKEN:
			// debug("status = connexion broken");
			goto close_socket;
			break;
		case REQ_TO_BE_CONT:
			// debug("status = to be continued");
			copy_request = new HttpRequest(request);
			sched_queue_task(tasks, fd, copy_request);
			continue;
		default:
			debug(YELLOW << "status = finished with:" << status_code << END);
			if (status_code == 0)
				debug(YELLOW << request.method << " " << request.url << " " << request.version << END);
			request.finished = true;
			request.status_code = status_code;
			// debug("queueing request");
			sched_queue_task(tasks, fd, new HttpRequest(request));
			watchlist_add_fd(wfd, fd, EVFILT_WRITE);
			// debug("queued request");
			continue;
			break;
		}
	response:
		// dump_request(request);
		// goto close_socket;
		finished =
			send_response(fd, request, response, request.status_code, &close_connexion);
		if (finished || close_connexion)
		{
			sched_unqueue_task(tasks, fd);
			if (close_connexion)
				goto close_socket;
			else
				watchlist_insert(wfd, fd);
		}
		else
		{
			sched_queue_task(tasks, fd, new HttpResponse(response));
		}

		continue;
	close_socket:
		debug(RED << "---" << END);
		// debug(RED << "closing socket" << END);
		sched_unqueue_task(tasks, fd);
		watchlist_del_fd(wfd, fd);
		close(fd);
	}
}

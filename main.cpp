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

int main(int argc, char **argv)
{
	if (argc != 2)
		die("usage: webserv <config_file>\n");

	signal(SIGPIPE, SIG_IGN);
	parse_config(argv[1]);
	// dump_config(config);

	int wfd = init_watchlist();
	spawn_servers(wfd);

	int finished = 0;
	std::map<int, SchedulableEntity *> tasks;
	bool close_connexion = false;

	while (1)
	{
		int status_code = 0;
		HttpRequest request;
		HttpResponse response;
		HttpRequest *copy_request;

		int fd = watchlist_wait_fd(wfd);

		if (fd > 2 && fd <= config.max_server_fd)
		{
			accept_connection(wfd, fd);
			continue;
		}
		// no new request, serve pending ones
		if (fd == WATCHL_NO_PENDING || tasks.find(fd) != tasks.end())
		{
			fd = sched_get_starved(tasks);
			if (fd == Q_EMPTY)
				continue;
			if (tasks[fd]->get_type() == REQUEST)
			{
				// debug("schedule pending requests");
				copy_request = dynamic_cast<HttpRequest *>(tasks[fd]);
				request = *copy_request;
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
			debug("received new request");

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
			// debug("status = finished with:" << status_code);
			if (status_code == 0)
				debug(request.method << " " << request.url << " " << request.version);
			sched_unqueue_task(tasks, fd);
			break;
		}
	response:
		// dump_request(request);
		// goto close_socket;
		finished =
			send_response(fd, request, response, status_code, &close_connexion);
		// std::cout<< YELLOW << "*********************>finished = "<< END << finished << std::endl;
		///////////////////////////////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////////////////////////////
		// finished = send_response(fd, request, response, status_code);
		// finished = send_response(fd, request, response, status_code, &close);
		// std::cout << close_connexion << "= " << std::endl;
		if (finished || close_connexion)
		{
			sched_unqueue_task(tasks, fd);
			if (close_connexion)
				goto close_socket;
		}
		else
		{
			sched_queue_task(tasks, fd, new HttpResponse(response));
		}

		continue;
	close_socket:
		debug("closing socket");
		sched_unqueue_task(tasks, fd);
		watchlist_del_fd(wfd, fd);
		close(fd);
	}
}






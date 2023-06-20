#ifdef __APPLE__
#include <sys/event.h>
#elif __linux__
#include <sys/epoll.h>
#endif
#include "webserv.hpp"

void die(std::string msg) {
	perror(msg.c_str());
	exit(1);
}

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


////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv) {
	if (argc != 2)
		die("usage: webserv <config_file>\n");

	parse_config(argv[1]);
	// dump_config(config);

    int wfd = init_watchlist();
	spawn_servers(wfd);

	int ret;

	std::map<int,HttpResponse>	clients;
	while (1) {
		int fd = watchlist_wait_fd(wfd);

		assert(fd >= 0);
		if (fd <= max_server_fd) {
			accept_connection(wfd, fd);
			continue;
		}

		std::string	http_rem;
		HttpRequest	request;

		int							status_code;
		HttpResponse				response;
		std::string					response_buffer;
		std::string					content_length;
		std::map<int, HttpResponse>::iterator clients_it = clients.find(fd);
		std::vector<Server>::iterator server_it;
		std::vector<Location>::iterator location_it;
		
		while (1) {
			char buffer[255];
			bool done = false;
			if ((ret = recv(fd, buffer, sizeof(buffer) - 1, 0)) < 0)
				goto close_socket;
			if (ret == 0)
				break;
			buffer[ret] = 0;
			http_rem += buffer;
			int parsed = parse_partial_http_request(http_rem, request, &done);
			if (parsed < 0) {
				status_code = -parsed;
				goto process;
			}
			http_rem.erase(0, parsed);
			if (done)
				break;
		}

		if (http_rem.length() || !http_req_valid(request)) {
			status_code = 400;
			goto process;
		}

		// request parsed
		// todo: check client_max_body
		server_it = server(config, request);
		location_it = location(config, request, server_it);

		if (location_it == server_it->routes.end()) {
			status_code = 404;
			goto process;
		}

		if (std::find(location_it->methods.begin(), location_it->methods.end(), request.method) == location_it->methods.end()) {
			status_code = 405;
			goto process;
		}

		std::cout << "--------- valid request received"<< std::endl;

		if (request.method == "GET")
			status_code = 1;
		if (request.method == "POST")
			status_code = 2;
		if (request.method == "DELETE")
			status_code = 3;

		// std::cout << "status_code " << status_code << std::endl;
		// status_code = parse_http_request(config, request_buffer, request);
		// if (status_code != 1 || status_code != 2 || status_code != 3)
		// {
		// 	response_Http_Request_error(status_code, config, response);
		// 	response_buffer = generate_http_response(response);
		// 	response_buffer += response.content;
		// 	send(fd, response_buffer.c_str(), response_buffer.length(), 0) ;
		// }
		////////////////////////////////////////////////////////////////////////////////////////
process:
		response.old_url = request.url;
		response.version = request.version;

		std::cout << "\033[32m"  << "method: " << request.method<< "\033[0m" << std::endl;
		std::cout << "\033[32m"  << "url: " << request.url<< "\033[0m" << std::endl;
		std::cout << "\033[32m"  << "version: " << request.version << "\033[0m" << std::endl;
		for (auto it = request.headers.begin(); it != request.headers.end(); it++) {
			std::cout << "\033[32m" << it->first << ' ' << it->second << "\033[0m" << std::endl;
		}
		///////////////////////////////////////////////////////////////////////////////////////////
		if (clients.empty() || clients_it == clients.end())
		{
			init_response(config, response, request, fd);
			if (status_code == 1)
			{
				if (response_get(config, response))
				{
					std::cout << "version --" << response.version << std::endl;
					content_length = read_File(response);
					if (content_length == "404")
					{
						ft_send_error(404, config, response);
						goto close_socket;
					}
					else
					{
						// response.headers["content-length"] = content_length;
						response.headers["Transfer-Encoding"] = "chunked";
						response_buffer = generate_http_response(response);
						// std::cout << "+++++++++++> " << response_buffer << std::endl;
						send(response.fd, response_buffer.c_str(), response_buffer.length(), 0);
						response.content = read_File(response);
						if (response.finish_reading)
						{
							send(response.fd, response.content.c_str(), response.content.length(), 0);
							goto close_socket;
						}
					}
				}
				else
					goto close_socket;
				// else
				// {
				// 	ft_send_error(status_code, config, response);
				// 	goto close_socket;
				// }
			}
			else if (status_code == 2)
			{
				if(!response_post(config, response))
					goto close_socket;
			}
			else if (status_code == 3)
			{
				if(!response_delete(config, response))
					goto close_socket;
			}
			else
			{
				ft_send_error(status_code, config, response);
				goto close_socket;
			}
			clients[fd] = response;
		}
		else
		{
			clients[fd].content = read_File(clients[fd]);
			send(fd, clients[fd].content.c_str(), clients[fd].content.length(), 0);
		}
		continue;

bad_request:
			std::cout << "--------- bad request"<< std::endl;
			exit(1);
close_socket:
			std::cout << "--------- invalid request: close socket"<< std::endl;
			clients.erase(fd);
			watchlist_del_fd(wfd, fd);
			close(fd);
	}
}

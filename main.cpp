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

		if (fd <= max_server_fd) {
			accept_connection(wfd, fd);
			continue;
		}

		std::string	request_buffer;
		HttpRequest	request;

		int							status_code;
		HttpResponse				response;
		std::string					response_buffer;
		std::string					content_length;
		std::map<int, HttpResponse>::iterator clients_it = clients.find(fd);

		while (1) {
			char buffer[255];
			// std::cout << "################ " << std::endl;
			if ((ret = recv(fd, buffer, sizeof buffer, 0)) < 0)
				goto close_socket;
			buffer[ret] = 0;
			request_buffer += buffer;
			if (ret != sizeof buffer)
				break;
		}

		if (request_buffer.length() == 0)
			goto close_socket;
		else
			std::cout << "--------- request received"<< std::endl;

		if (parse_http_request(request_buffer, request) < 0)
			status_code = 400;
		else
		{
			response.old_url = request.url;
			status_code = check_req_line_headers(config, request);
		}
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
close_socket:
			std::cout << "--------- invalid request: close socket"<< std::endl;
			clients.erase(fd);
			watchlist_del_fd(wfd, fd);
			close(fd);
	}
}

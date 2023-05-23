#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string>
#include <iostream>
#include <sys/epoll.h>
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

void spawn_servers(int efd) {
	for (size_t i = 0; i < config.servers.size(); i++) {
		int sock = socket(PF_INET, SOCK_STREAM, 0);
		if (sock < 0)
			die("socket");

		struct sockaddr_in address;
		address.sin_family = AF_INET;
		address.sin_addr.s_addr = inet_addr(config.servers[i].ip);
		address.sin_port = htons(config.servers[i].port);

		int ret = bind(sock, (struct sockaddr *)&address, sizeof(address));
		if (ret < 0)
			die("bind");

		ret = listen(sock, BACKLOG_SIZE);
		if (ret < 0)
			die("listen");
		std::cout << "--------- " << "listening on: " << config.servers[i].ip << ":" << config.servers[i].port << std::endl;

		epoll_add_fd(efd, sock, EPOLLIN);
		max_server_fd = sock;
	}
}

void accept_connection(int efd, int server) {
	struct sockaddr_in caddress;
	socklen_t len = sizeof caddress;
	int client = accept(server, (struct sockaddr *)&caddress, &len);
	if (client < 0)
		die("accept");
	
	epoll_add_fd(efd, client, EPOLLIN);

	std::cout << "--------- " << "connection received " << inet_ntoa(caddress.sin_addr) << ":" << ntohs(caddress.sin_port) << std::endl;
}

void parse_config(std::string config_file) {
	// TODO:
}

int main(int argc, char **argv) {
	if (argc != 2)
		die("%s config\n", argv[0]);
	parse_config(argv[1]);
    int efd = init_epoll();
	spawn_servers(efd);

	while (1) {
		int fd = epoll_wait_fd(efd);

		if (fd <= max_server_fd) {
			accept_connection(efd, fd);
			continue;
		}

		std::string request;
		while (1) {
			char buffer[255];
			ret = recv(fd, buffer, sizeof buffer, 0);
			if (ret <= 0)
				die("read");
			buffer[ret] = 0;
			request += buffer;
			if (ret != sizeof buffer)
				break;
		}

		HttpRequest req;
		parse_http_request(request, req);

		// std::cout << "--------- " << "method: " << req.method << std::endl;
		// std::cout << "--------- " << "url: " << req.url << std::endl;
		// std::cout << "--------- " << "version: " << req.version << std::endl;

		// for (auto it = req.headers.begin(); it != req.headers.end(); it++) {
		// 	std::cout << "--------- " << it->first << ' ' << it->second << std::endl;
		// }

		HttpResponse res;
		res.code = 200;
		res.content = "ft_webserv yora7ibo bikom";
		res.headers["Content-Type"] = "text/html";
		std::string res_str = generate_http_response(res);
		ret = send(fd, res_str.c_str(), res_str.length(), 0);
		if (ret < 0)
			die("send");
	}
	
	close(efd);
	close(sock);
}

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string>
#include <iostream>
#include "webserv.hpp"

void die(std::string msg) {
	std::perror(msg.c_str());
	exit(1);
}

int main(int argc, char **argv) {
	int sock = socket(PF_INET, SOCK_STREAM, 0);
	if (sock < 0)
		die("socket");

	struct sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);

	int ret = bind(sock, (struct sockaddr *)&address, sizeof(address));
	if (ret < 0)
		die("bind");

	ret = listen(sock, BACKLOG_SIZE);
	if (ret < 0)
		die("listen");

	std::cout << "listening on port: " << PORT << std::endl;
	struct sockaddr_in caddress;
	socklen_t len = sizeof caddress;
	int client = accept(sock, (struct sockaddr *)&caddress, &len);
	if (client < 0)
		die("accept");

	char str[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &caddress.sin_addr, str, INET_ADDRSTRLEN);
	std::cout << "connection received " << str << ":" << ntohs(caddress.sin_port) << std::endl;

	std::string http_head;
	while (1) {
		char buffer[255];
		ret = recv(client, buffer, sizeof buffer, 0);
		if (ret < 0)
			die("read");
		buffer[ret] = 0;
		http_head += buffer;
		if (http_head.find(HTTP_DEL HTTP_DEL) != std::string::npos)
			break;
	}

	HttpRequest req;
	parse_http_request(http_head, req);

	std::cout << "method: " << req.method << std::endl;
	std::cout << "url: " << req.url << std::endl;
	std::cout << "version: " << req.version << std::endl;

	for (auto it = req.headers.begin(); it != req.headers.end(); it++) {
   		std::cout << it->first << '\t' << it->second << std::endl;
	}

	HttpResponse res;
	res.code = 200;
	res.content = "ft_webserv yor7ibo bikom";
	res.headers["Content-Type"] = "text/html";
	std::string res_str = generate_http_response(res);
	ret = send(client, res_str.c_str(), res_str.length(), 0);
	if (ret < 0)
		die("send");
	
	close(client);
	close(sock);
}

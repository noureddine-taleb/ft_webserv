#include <iostream>
#include <string>
#include <map>
#include <sstream>
#include <vector>
#include "webserv.hpp"

void handle_http_response(const HttpRequest &req, HttpResponse &res) {
	(void)req;
	// std::cout << "--------- " << "method: " << req.method << std::endl;
	// std::cout << "--------- " << "url: " << req.url << std::endl;
	// std::cout << "--------- " << "version: " << req.version << std::endl;

	// for (auto it = req.headers.begin(); it != req.headers.end(); it++) {
	// 	std::cout << "--------- " << it->first << ' ' << it->second << std::endl;
	// }

	res.code = 200;
	res.content = "{ \"message\": \"ft_webserv yora7ibo bikom\" }";
	res.headers["Content-Type"] = "text/json";
}
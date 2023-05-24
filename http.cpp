#include <iostream>
#include <string>
#include <map>
#include <sstream>
#include <vector>
#include "webserv.hpp"

// request

// GET /name HTTP/1.1
// Host: localhost:8080
// Connection: keep-alive
// Cache-Control: max-age=0
// sec-ch-ua: "Chromium";v="112", "Google Chrome";v="112", "Not:A-Brand";v="99"
// sec-ch-ua-mobile: ?0
// sec-ch-ua-platform: "macOS"
// Upgrade-Insecure-Requests: 1
// User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/112.0.0.0 Safari/537.36
// Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7
// Sec-Fetch-Site: none
// Sec-Fetch-Mode: navigate
// Sec-Fetch-User: ?1
// Sec-Fetch-Dest: document
// Accept-Encoding: gzip, deflate, br
// Accept-Language: en-US,en;q=0.9
// If-None-Match: "64230162-267"
// If-Modified-Since: Tue, 28 Mar 2023 15:01:54 GMT

// Response

// HTTP/1.1 304 Not Modified
// Server: nginx/1.23.4
// Date: Thu, 11 May 2023 13:40:26 GMT
// Last-Modified: Tue, 28 Mar 2023 15:01:54 GMT
// Connection: keep-alive
// ETag: "64230162-267"


void parse_http_request(std::string req_str, HttpRequest &req) {
    std::vector<std::string> vec = split(req_str, HTTP_DEL);
    std::vector<std::string> headv = split(vec[0], " ");
	size_t content_off = 0;

	req.method = headv[0];
	req.url = headv[1];
	req.version = headv[2];
	assert(req_str.find(HTTP_DEL HTTP_DEL) != std::string::npos);
	content_off = req_str.find(HTTP_DEL HTTP_DEL) + sizeof(HTTP_DEL HTTP_DEL) - 1;

	vec.erase(vec.begin());

    for (size_t i = 0; i < vec.size(); i++) {
		std::string line = vec[i];
		if (line == "") // end of headers
			break;
   		std::vector<std::string> headerv = split(line, ":", 1);
		req.headers[headerv[0]] = headerv[1];
	}
	req.content = req_str.substr(content_off);
}

std::string generate_http_response(HttpResponse &res) {
	std::stringstream res_str;

    time_t t = time(NULL);
    char mbstr[100];
 
    strftime(mbstr, sizeof(mbstr), "%a, %d %b %Y %T GMT", localtime(&t));

	res.headers["Server"] = "ft_webserv/0.1";
	res.headers["Date"] = mbstr;
	res.headers["Connection"] = "keep-alive";
	res.headers["Content-Length"] = std::to_string(res.content.length());

	// Last-Modified: Tue, 28 Mar 2023 15:01:54 GMT
	// ETag: "64230162-267"

	res_str << "HTTP/1.1" << " " << res.code << "CODE DESC" << HTTP_DEL;
	for (auto it = res.headers.begin(); it != res.headers.end(); it++) {
		res_str << it->first << ": " << it->second << HTTP_DEL;
	}

	res_str << HTTP_DEL;
	res_str << res.content;
	return res_str.str();
}
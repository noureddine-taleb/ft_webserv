#include <iostream>
#include <string>
#include <map>
#include <sstream>
#include <vector>
#include <cstring>
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

// static std::string http_codes[] = {
// 	[100] = "Continue",
// 	[101] = "Switching Protocols",
// 	[102] = "Processing",

// 	[200] = "OK",
// 	[201] = "Created",
// 	[202] = "Accepted",
// 	[203] = "Non-Authoritative Information",
// 	[204] = "No Content",
// 	[205] = "Reset Content",
// 	[206] = "Partial Content",
// 	[207] = "Multi-Status",
// 	[208] = "Already Reported",
// 	[226] = "IM Used",

// 	[300] = "Multiple Choices",
// 	[301] = "Moved Permanently",
// 	[302] = "Found",
// 	[303] = "See Other",
// 	[304] = "Not Modified",
// 	[305] = "Use Proxy",
// 	[307] = "Temporary Redirect",
// 	[308] = "Permanent Redirect",

// 	[400] = "Bad Request",
// 	[401] = "Unauthorized",
// 	[402] = "Payment Required",
// 	[403] = "Forbidden",
// 	[404] = "Not Found",
// 	[405] = "Method Not Allowed",
// 	[406] = "Not Acceptable",
// 	[407] = "Proxy Authentication Required",
// 	[408] = "Request Timeout",
// 	[409] = "Conflict",
// 	[410] = "Gone",
// 	[411] = "Length Required",
// 	[412] = "Precondition Failed",
// 	[413] = "Request Entity Too Large",
// 	[414] = "Request-URI Too Long",
// 	[415] = "Unsupported Media Type",
// 	[416] = "Requested Range Not Satisfiable",
// 	[417] = "Expectation Failed",
// 	[418] = "I'm a teapot",
// 	[420] = "Enhance Your Calm",
// 	[422] = "Unprocessable Entity",
// 	[423] = "Locked",
// 	[424] = "Failed Dependency",
// 	[425] = "Reserved for WebDAV",
// 	[426] = "Upgrade Required",
// 	[428] = "Precondition Required",
// 	[429] = "Too Many Requests",
// 	[431] = "Request Header Fields Too Large",
// 	[444] = "No Response",
// 	[449] = "Retry With",
// 	[450] = "Blocked by Windows Parental Controls",
// 	[451] = "Unavailable For Legal Reasons",
// 	[499] = "Client Closed Request",

// 	[500] = "Internal Server Error",
// 	[501] = "Not Implemented",
// 	[502] = "Bad Gateway",
// 	[503] = "Service Unavailable",
// 	[504] = "Gateway Timeout",
// 	[505] = "HTTP Version Not Supported",
// 	[506] = "Variant Also Negotiates",
// 	[507] = "Insufficient Storage",
// 	[508] = "Loop Detected",
// 	[509] = "Bandwidth Limit Exceeded",
// 	[510] = "Not Extended",
// 	[511] = "Network Authentication Required",
// 	[598] = "Network read timeout error",
// 	[599] = "Network connect timeout error",
// };

enum {
	BadRequest = 400,
	RequestURITooLong = 414,
	MethodNotAllowed = 405,
	NotImplemented = 501,
	RequestEntityTooLarge = 413
};

// todo split this gigantic func
int parse_partial_http_request(HttpRequest &request, bool *done) {
	// parse http header
	int parsed = 0;
	for (; request.http_buffer.size(); request.http_buffer.erase(request.http_buffer.begin(), request.http_buffer.begin() + parsed)) {
		parsed = 0;
		if (!request.__http_headers_end && find(HTTP_DEL, request.http_buffer) == request.http_buffer.end())
			return 0;
		if (!request.__http_top_header_parsed) {
			std::string http_line (request.http_buffer.begin(), find(HTTP_DEL, request.http_buffer));
			std::vector<std::string> http_headerv = split(http_line, " ");
			if (http_headerv.size() != 3)
				return -BadRequest;

			request.method = http_headerv[0];
			request.url = http_headerv[1];
			request.version = http_headerv[2];

			if (request.method != "GET" && request.method != "POST" && request.method != "DELETE")
				return -MethodNotAllowed;
			if (request.url.find_first_not_of("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~:/?#[]@!$&'()*+,;=%") != std::string::npos)
				return -BadRequest;
			if (request.url.length() > 2048)
				return (-RequestURITooLong);
			// todo: maybe other versions should be supported
			if (request.version != "HTTP/1.1")
				return -BadRequest;
			request.__http_top_header_parsed = true;
			parsed += http_line.length() + HTTP_DEL_LEN;
		} else if (!request.__http_headers_end) {
			std::string http_line (request.http_buffer.begin(), find(HTTP_DEL, request.http_buffer));
			// end of headers
			if (http_line.length() == 0) {
				if (!request.headers.count("Host"))
					return -BadRequest;
				// for post at least one header should be specified: Transfer-Encoding or Content-Length
				if (request.method == "POST" && !request.headers.count("Transfer-Encoding") && !request.headers.count("Content-Length"))
					return -BadRequest;
				request.__http_headers_end = true;
				parsed += HTTP_DEL_LEN;
				continue;
			}
			std::vector<std::string> headerv = split(http_line, ":", 1);
			if (headerv.size() != 2)
				return -BadRequest;
			headerv[0] = trim(headerv[0]);
			headerv[1] = trim(headerv[1]);
			if (headerv[0].length() == 0 || headerv[1].length() == 0)
				return -BadRequest;
			if (headerv[0] == "Transfer-Encoding" && headerv[1] != "chunked")
				return -NotImplemented;
			if (headerv[0] == "Content-Length") {
				try {
					int size = std::stoi(headerv[1]);
					if (size < 0)
						return -BadRequest;
					if (size > config.client_max_body_size)
						return -RequestEntityTooLarge;
				} catch(std::invalid_argument) {
					return -BadRequest;
				}
			}
			request.headers[headerv[0]] = headerv[1];
			parsed += http_line.length() + HTTP_DEL_LEN;
		} else {
			if (request.method != "POST") {
				*done = true;
				return 0;
			}
			if (request.headers["Transfer-Encoding"] == "chunked") {
				if (find(HTTP_DEL, request.http_buffer) == request.http_buffer.end())
					return 0;
				int size;
				std::vector<char> chunk_size(request.http_buffer.begin(), find(HTTP_DEL, request.http_buffer));
				try {
					size = std::stoi(std::string(chunk_size.begin(), chunk_size.end()), 0, 16);
				} catch (std::invalid_argument) {
					return -BadRequest;
				}
				std::vector<char> chunk(find(HTTP_DEL, request.http_buffer) + HTTP_DEL_LEN, request.http_buffer.end());
				if (chunk.size() < (size + HTTP_DEL_LEN))
					return 0;
				if (size == 0) {
					*done = true;
					return parse_form_data_files(request) == 0 ? 0 : -BadRequest;
				}
				chunk = std::vector<char>(chunk.begin(), chunk.begin() + size);
				request.content.insert(request.content.end(), chunk.begin(), chunk.end());
				if (request.content.size() > (unsigned int)config.client_max_body_size)
					return -RequestEntityTooLarge;
				parsed += chunk_size.size() + HTTP_DEL_LEN + chunk.size() + HTTP_DEL_LEN;
			} else {
				unsigned int size = std::stoi(request.headers["Content-Length"]);
				if (size > (unsigned int)config.client_max_body_size)
					return -RequestEntityTooLarge;
				unsigned int len = request.http_buffer.size();
				unsigned int rem = size - request.content.size();
				if (len > rem) {
					len = rem;
					debug("content size bigger than Content-Length\n");
				}
				request.content.insert(request.content.end(), request.http_buffer.begin(), request.http_buffer.begin() + len);
				parsed += len;
				if (request.content.size() >= size) {
					*done = true;
					return parse_form_data_files(request) == 0 ? 0 : -BadRequest;
				}
			}
		}
	}
	return 0;
}

/**
 * get and parse http request from fd
 * @return:
 * 0: success
 * -1: connection is broken and should be closed
 * -2: request not finished yet: to be continued
 * x: http failure status code (4xx, 5xx)
*/
int get_request(int fd, HttpRequest &request) {
	int ret;
	char buffer[255];
	bool done;
	int iter = 0;
	int max_iter = 5;

	while (1) {
		done = false;
		// todo: check if the socket would block
		if ((ret = recv(fd, buffer, sizeof(buffer) - 1, 0)) < 0)
			return REQ_CONN_BROKEN;
		if (ret == 0) {
			debug("recv == 0\n");
			return REQ_CONN_BROKEN;
		}
		int last_size = request.http_buffer.size();
		request.http_buffer.resize(last_size + ret);
		memcpy(&request.http_buffer[last_size], buffer, ret);
		int ret = parse_partial_http_request(request, &done);
		if (ret < 0)
			return -ret;
		if (done)
			break;
		iter++;
		if (iter >= max_iter)
			return REQ_TO_BE_CONT;
	}
	return 0;
}

void dump_request(HttpRequest &request) {
	std::cout << "method: " << request.method << std::endl;
	std::cout << "url: " << request.url << std::endl;
	std::cout << "version: " << request.version << std::endl;
	for (std::map<std::string, std::string>::iterator it = request.headers.begin(); it != request.headers.end(); it++) {
		std::cout << it->first << " : " << it->second << std::endl;
	}

	std::cout << std::endl;
	std::cout << "+++++++++++++++++++++++++++++++++" << std::endl;
	for (std::vector<char>::iterator it = request.content.begin(); it != request.content.end(); it++) {
		std::cout << *it;
	}
	std::cout << "+++++++++++++++++++++++++++++++++" << std::endl;

	std::cout << "fffffffffffffffffffffffffffffffff" << std::endl;
	for (std::vector<File>::iterator it = request.files.begin(); it != request.files.end(); it++) {
		std::cout << it->name << std::endl;
		for (std::vector<char>::iterator it2 = it->content.begin(); it2 != it->content.end(); it2++) {
			std::cout << *it2;
		}
		std::cout << std::endl;
	}
	std::cout << "fffffffffffffffffffffffffffffffff" << std::endl;
}
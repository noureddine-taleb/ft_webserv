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

enum
{
	BadRequest = 400,
	RequestURITooLong = 414,
	MethodNotAllowed = 405,
	NotImplemented = 501,
	RequestEntityTooLarge = 413
};

int parse_http_top_header(HttpRequest &request, int &parsed) {
	std::string http_line(request.http_buffer.begin(), find(HTTP_DEL, request.http_buffer));
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
	if (request.version != "HTTP/1.1")
		debug("http protocol is not HTTP/1.1");
	request.__http_top_header_parsed = true;
	parsed += http_line.length() + HTTP_DEL_LEN;
	return 0;
}

int parse_http_headers(HttpRequest &request, int &parsed, bool *done) {
	std::string http_line(request.http_buffer.begin(), find(HTTP_DEL, request.http_buffer));
	// end of headers
	if (http_line.length() == 0)
	{
		if (!request.headers.count("Host"))
			return -BadRequest;
		// for post at least one header should be specified: Transfer-Encoding or Content-Length
		if (request.method == "POST" && !request.headers.count("Transfer-Encoding") && !request.headers.count("Content-Length"))
			return -BadRequest;
		request.__http_headers_end = true;
		parsed += HTTP_DEL_LEN;
		if (request.method == "GET" || request.method == "DELETE")
			*done = true;
		return 0;
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
	if (headerv[0] == "Content-Length")
	{
		try
		{
			int size = ft_stoi(headerv[1]);
			if (size < 0)
				return -BadRequest;
			if (size > config.client_max_body_size)
				return -RequestEntityTooLarge;
		}
		catch (std::invalid_argument &)
		{
			return -BadRequest;
		}
	}
	request.headers[headerv[0]] = headerv[1];
	parsed += http_line.length() + HTTP_DEL_LEN;
	return 0;
}

int parse_http_body(HttpRequest &request, int &parsed, bool *done) {
	if (request.headers["Transfer-Encoding"] == "chunked")
	{
		if (find(HTTP_DEL, request.http_buffer) == request.http_buffer.end())
			return 0;
		unsigned int size;
		std::vector<char> chunk_size(request.http_buffer.begin(), find(HTTP_DEL, request.http_buffer));
		std::string chunk_size_s(chunk_size.begin(), chunk_size.end());
		try
		{
			size = ft_stoi_base_16(chunk_size_s);
		}
		catch (std::invalid_argument &)
		{
			return -BadRequest;
		}
		std::vector<char> chunk(find(HTTP_DEL, request.http_buffer) + HTTP_DEL_LEN, request.http_buffer.end());
		if (chunk.size() < (size + HTTP_DEL_LEN))
			return 0;
		if (size == 0)
		{
			*done = true;
			return parse_form_data_files(request) == 0 ? 0 : -BadRequest;
		}
		chunk = std::vector<char>(chunk.begin(), chunk.begin() + size);
		request.content.insert(request.content.end(), chunk.begin(), chunk.end());
		if (request.content.size() > (unsigned int)config.client_max_body_size)
			return -RequestEntityTooLarge;
		parsed += chunk_size.size() + HTTP_DEL_LEN + chunk.size() + HTTP_DEL_LEN;
	}
	else
	{
		unsigned int size = ft_stoi(request.headers["Content-Length"]);
		if (size > (unsigned int)config.client_max_body_size)
			return -RequestEntityTooLarge;
		unsigned int len = request.http_buffer.size();
		unsigned int rem = size - request.content.size();
		if (len > rem)
		{
			len = rem;
			debug("content size bigger than Content-Length\n");
		}
		request.content.insert(request.content.end(), request.http_buffer.begin(), request.http_buffer.begin() + len);
		parsed += len;
		if (request.content.size() >= size)
		{
			*done = true;
			return parse_form_data_files(request) == 0 ? 0 : -BadRequest;
		}
	}
	return parsed;
}

int parse_partial_http_request(HttpRequest &request, bool *done)
{
	// parse http header
	int parsed = 0;
	for (; request.http_buffer.size(); request.http_buffer.erase(request.http_buffer.begin(), request.http_buffer.begin() + parsed))
	{
		parsed = 0;
		if (!request.__http_headers_end && find(HTTP_DEL, request.http_buffer) == request.http_buffer.end())
			return 0;
		if (!request.__http_top_header_parsed)
		{
			int ret = parse_http_top_header(request, parsed);
			if (ret < 0)
				return ret;
		}
		else if (!request.__http_headers_end)
		{
			int ret = parse_http_headers(request, parsed, done);
			if (ret < 0 || *done)
				return ret;
		}
		else
		{
			int ret = parse_http_body(request, parsed, done);
			if (ret <= 0 || *done)
				return ret;
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
int get_request(int fd, HttpRequest &request)
{
	int ret;
	char buffer[1024];
	bool done = false;

	if (check_connexion(fd) < 0)
		return REQ_CONN_BROKEN;

	ret = recv(fd, buffer, sizeof(buffer) - 1, 0);
	// EAGAIN case
	if (ret < 0)
		return REQ_TO_BE_CONT;
	// connexion broken
	if (ret == 0)
		return REQ_CONN_BROKEN;
	int last_size = request.http_buffer.size();
	request.http_buffer.resize(last_size + ret);
	std::memcpy(&request.http_buffer[last_size], buffer, ret);
	ret = parse_partial_http_request(request, &done);
	if (ret < 0)
		return -ret;
	if (done)
		return 0;
	return REQ_TO_BE_CONT;
}

void dump_request(HttpRequest &request)
{
	std::cout << GREEN;
	std::cout << "method: " << request.method << std::endl;
	std::cout << "url: " << request.url << std::endl;
	std::cout << "version: " << request.version << std::endl;
	for (std::map<std::string, std::string>::iterator it = request.headers.begin(); it != request.headers.end(); it++)
	{
		std::cout << it->first << " : " << it->second << std::endl;
	}

	std::cout << std::endl;
	std::cout << "++++++++++++++++raw body+++++++++++++++++" << std::endl;
	// for (std::vector<char>::iterator it = request.content.begin(); it != request.content.end(); it++)
	// {
	// 	std::cout << *it;
	// }
	std::cout << "+++++++++++++++++++++++++++++++++" << std::endl;

	std::cout << "ffffffffffffffff_parsed_files_fffffffffffffffff" << std::endl;
	for (std::vector<File>::iterator it = request.files.begin(); it != request.files.end(); it++)
	{
		std::cout << it->name << std::endl;
		for (std::vector<char>::iterator it2 = it->content.begin(); it2 != it->content.end(); it2++)
			std::cout << *it2;
		std::cout << std::endl;
	}
	std::cout << "fffffffffffffffffffffffffffffffff" << std::endl;

	std::cout << "vvvvvvvvvvvvvvvv_parsed_vars_vvvvvvvvvvvvvvvvv" << std::endl;
	for (std::vector<Var>::iterator it = request.vars.begin(); it != request.vars.end(); it++)
	{
		std::cout << it->key << std::endl;
		for (std::vector<char>::iterator it2 = it->value.begin(); it2 != it->value.end(); it2++)
			std::cout << *it2;
		std::cout << std::endl;
	}
	std::cout << "vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv" << std::endl;
	std::cout << END;
}

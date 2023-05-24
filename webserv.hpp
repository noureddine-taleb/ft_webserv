#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include <string>
#include <map>
#include <vector>
#include "config.hpp"

#define BACKLOG_SIZE 32
#define HTTP_DEL "\r\n"
// #define assert(cond) if (!(cond)) \
// 						die("assertion failed: " #cond);

/*
	execve, dup, dup2, pipe, strerror, gai_strerror,
	errno, dup, dup2, fork, htons, htonl, ntohs, ntohl,
	select, poll, epoll (epoll_create, epoll_ctl,
	epoll_wait), kqueue (kqueue, kevent), socket,
	accept, listen, send, recv, bind, connect,
	getaddrinfo, freeaddrinfo, setsockopt, getsockname,
	getprotobyname, fcntl, close, read, write,
	waitpid, kill, signal, access, opendir, readdir
	and closedir.
*/

class HttpRequest {
	public:
		std::string method;
		std::string url;
		std::string version;
		std::map<std::string, std::string> headers;

		std::string content;
};

class HttpResponse {
	public:
		std::string version;
		int code;
		std::map<std::string, std::string> headers;

		std::string content;
};

void die(std::string msg);

// http
std::vector<std::string> split(std::string s, std::string delimiter, unsigned int max_splits = -1);
void parse_http_request(std::string req_str, HttpRequest &req);
std::string generate_http_response(HttpResponse &res);

// epoll
int init_watchlist();
void watchlist_add_fd(int efd, int fd, uint32_t events);
void watchlist_del_fd(int efd, int fd);
int watchlist_wait_fd(int efd);

#endif // WEBSERV

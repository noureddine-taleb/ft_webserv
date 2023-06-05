#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include <string>
#include <map>
#include <vector>
#include <cassert>
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
		std::string reason_phrase;
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
std::string trim(std::string s);
void parse_config(std::string config_file);
void dump_config(Config config);
void handle_http_response(const HttpRequest &req, HttpResponse &res);
//----------------------------------------------------------------------------
int				check_req_well_formed(HttpRequest &req,Config& config);
void			response_Http_Request(int status_code, HttpRequest& request, Config& config, HttpResponse& response);
HttpResponse	response_Http_Request_error(int status_code, HttpRequest& request, Config& config);
std::string		res_content(int status_code, HttpRequest& request, Config& config);
std::vector<Server>::iterator server(Config& config, HttpRequest& request);
std::string		read_File(std::string Path);
int				ft_atoi(std::string s);
int				response_get(HttpRequest& req, Config& config);

#endif // WEBSERV

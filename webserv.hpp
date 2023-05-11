#ifndef WEBSERV
#define WEBSERV

#include <string>
#include <map>

#define PORT 8082
#define BACKLOG_SIZE 32
#define HTTP_DEL "\r\n"
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
};

class HttpResponse {
	public:
		std::string version;
		int code;
		std::string content;
		std::map<std::string, std::string> headers;
};

std::vector<std::string> split(std::string s, std::string delimiter, unsigned int max_splits = -1);
void parse_http_request(std::string req_str, HttpRequest &req);
std::string generate_http_response(HttpResponse &res);

#endif // WEBSERV

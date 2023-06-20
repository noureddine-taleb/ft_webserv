#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <cassert>
#include <sys/stat.h>
#include "config.hpp"
#include <dirent.h>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cassert>
#include <fstream>
#include <sstream>

#define BACKLOG_SIZE 32
#define HTTP_DEL "\r\n"
#define BUFF_SIZE 8419815
// #define assert(cond) if (!(cond)) \
// die("assertion failed: " #cond);

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
		HttpRequest() : method(""), url(""), version(""), content(""), __http_top_header_parsed(false), __http_headers_end(false) {}
		std::string method;
		std::string url;
		std::string version;
		std::map<std::string, std::string> headers;
		std::string content;

		// pcb stuff
		bool __http_top_header_parsed;
		bool __http_headers_end;
};

class HttpResponse {
	public:
		HttpResponse () : get_length(false), finish_reading(false){}
		std::string version;
		int code;
		std::string reason_phrase;
		std::map<std::string, std::string> headers;
		std::string content;
		std::vector<Server>::iterator server_it;
		std::vector<Location>::iterator location_it;
		std::string    path_file;
		std::string old_url;
		bool get_length;
		int size_file;
		bool finish_reading;
		HttpRequest request;
		int byte_reading;
		int	fd;
};

void die(std::string msg);

// http
std::vector<std::string> split(std::string s, std::string delimiter, unsigned int max_splits = -1);
int parse_http_request(std::string req_str, HttpRequest &req);
int parse_partial_http_request(std::string req_str, HttpRequest &req);
// int parse_http_request(Config config, std::string req_str, HttpRequest &req);
std::string generate_http_response(HttpResponse &res);
int http_req_valid(HttpRequest &req);

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

std::vector<Server>::iterator server(Config& config, HttpRequest& request);
std::vector<Location>::iterator	location(Config& config, HttpRequest& req, std::vector<Server>::iterator server);
int				check_req_line_headers(Config& config, HttpRequest &request);
void			response_Http_Request(int status_code, HttpRequest& request, Config& config, HttpResponse& response, std::string path);
void			response_Http_Request_error(int status_code, Config& config, HttpResponse& response);
std::string		res_content(int status_code, Config& config, HttpResponse& response);
std::string		read_File_error(std::string Path);
int				ft_atoi(std::string s);
int				response_get(Config& config, HttpResponse& response);
std::string get_content_type(std::string path);
std::string		type_repo(std::string path);
std::string		content_dir(std::string dir,HttpResponse& response, std::vector<std::string>& content);
int				res_content_dir(int status_code, Config& config, HttpResponse& response);
std::string		res_content_file(int status_code, HttpRequest& request, Config& config, HttpResponse& response, std::string path);
std::string		read_File(HttpResponse& response);
void			ft_send_error(int status_code, Config config, HttpResponse& response);
void			init_response(Config& config, HttpResponse& response, HttpRequest& request, int fd);
void			fill_response(int status_code, HttpResponse& response);
int				get_path(Config config, HttpResponse& response);
std::string		get_reason_phase(int status_code);
std::string		ft_tostring(int nbr);
int				response_redirect(HttpResponse& response, Config& config);
int response_Http_Request(int status_code , Config& config, HttpResponse& response);
std::string	generate_filename();
int response_post(Config& config, HttpResponse& response);
void	add_extention(std::string& filename,HttpResponse& response);
void	upload_exist(Config& config, HttpResponse& response, std::string& upload_path);
void	upload_not_exist(Config& config, HttpResponse& response);
int response_delete(Config& config, HttpResponse& response);
// int			check_req_well_formed(int fd,Config& config, std::map<int,HttpResponse>& responses);
// std::string	read_File(std::map<int,HttpResponse>& responses, int fd );
// void			response_get(int fd, Config& config, std::map<int,HttpResponse>& responses);


#endif // WEBSERV

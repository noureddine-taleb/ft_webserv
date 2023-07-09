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
#include "sched.hpp"
#include <dirent.h>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cassert>
#include <fstream>
#include <sstream>

#define BACKLOG_SIZE 32
#define HTTP_DEL "\r\n"
#define HTTP_DEL_LEN (sizeof(HTTP_DEL) - 1)
#define BUFF_SIZE 1000000 // 1

#define WATCHL_NO_PENDING 0
#define REQ_CONN_BROKEN -1
#define REQ_TO_BE_CONT -2

#define debug(msg) std::cerr << "*********************" << __FILE__ << ":" << __LINE__ << " " << msg << std::endl

#define die(msg)	do {					\
		perror(std::string(msg).c_str());	\
		std::cout <<std::endl << strerror(errno) << std::endl << std::endl; \
		throw std::runtime_error("recv FAILED"); \
		exit(1);							\
	} while (0);

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

class File {
public:
	std::string name;
	std::vector<char> content;
};

class HttpRequest: public SchedulableEntity {
	public:
		HttpRequest() : method(""), url(""), version(""), __http_top_header_parsed(false), __http_headers_end(false) {}
		std::string method;
		std::string url;
		std::string version;
		std::map<std::string, std::string> headers;
		std::vector<char> content;

		// derived from content
		std::vector<File> files;

		// pcb stuff
		std::vector<char>	http_buffer;
		bool __http_top_header_parsed;
		bool __http_headers_end;
		enum SchedulableEntityTypes get_type() {
			return REQUEST;
		}
};

class HttpResponse: public SchedulableEntity {
	public:
		HttpResponse () : get_length(false), finish_reading(false){}
		std::string version;
		int code;
		std::string reason_phrase;
		std::map<std::string, std::string> headers;
		std::vector<char> content;
		std::string content_error;
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
		std::string query_str;
		int	url_changed;
		enum SchedulableEntityTypes get_type() {
			return RESPONSE;
		}
};

extern Config config;

// void die(std::string msg);

// socket
void spawn_servers(int wfd);
void accept_connection(int wfd, int server);
int get_request(int fd, HttpRequest &request);

// http
int parse_partial_http_request(HttpRequest &req, bool *done);
std::string generate_http_response(HttpResponse &res);
int parse_form_data_files(HttpRequest &request);

// epoll
int init_watchlist();
void watchlist_add_fd(int efd, int fd, uint32_t events);
void watchlist_del_fd(int efd, int fd);
int watchlist_wait_fd(int efd);
void parse_config(std::string config_file);
void dump_config(Config config);

// lib
std::vector<char>::iterator find(std::string str, std::vector<char> &vec);
std::string trim(std::string s);
std::vector<std::string> split(std::string s, std::string delimiter, unsigned int max_splits = -1);

//----------------------------------------------------------------------------

std::vector<Server>::iterator server(HttpRequest& request);
std::vector<Location>::iterator	location(HttpRequest& req, std::vector<Server>::iterator server);
int				check_req_line_headers(HttpRequest &request);
void			response_Http_Request(int status_code, HttpRequest& request, HttpResponse& response, std::string path);
void			response_Http_Request_error(int status_code, HttpResponse& response);
std::string		res_content(int status_code, HttpResponse& response);
std::string		read_File_error(std::string Path);
int				ft_atoi(std::string s);
int				response_get(HttpResponse& response);
std::string get_content_type(std::string path);
std::string		type_repo(std::string path);
std::string		content_dir(std::string dir,HttpResponse& response, std::vector<std::string>& content);
int				res_content_dir(int status_code, HttpResponse& response);
std::string		res_content_file(int status_code, HttpRequest& request, HttpResponse& response, std::string path);
// std::string		read_File(HttpResponse& response);
void			read_File(HttpResponse& response);
void			ft_send_error(int status_code, HttpResponse& response);
void			init_response(HttpResponse& response, HttpRequest& request, int fd);
void			fill_response(int status_code, HttpResponse& response);
int				get_path(HttpResponse& response);
std::string		get_reason_phase(int status_code);
std::string		ft_tostring(int nbr);
int				response_redirect(HttpResponse& response);
int response_Http_Request(int status_code , HttpResponse& response);
std::string	generate_filename();
int response_post(HttpResponse& response);
void	add_extention(std::string& filename,HttpResponse& response);
void	upload_exist(HttpResponse& response, std::string& upload_path);
void	upload_not_exist(HttpResponse& response);
int response_delete(HttpResponse& response);
// int continue_previous_response(HttpResponse &response);
// int new_request(HttpRequest &request);
int	send_response(int fd, HttpRequest& request, HttpResponse& response, int status_code, bool *close_connexion);
int new_request(HttpRequest &request, HttpResponse &response, int status_code);
int continue_previous_response(HttpResponse &response) ;
void dump_request(HttpRequest &request);
int	response_rewrite(HttpResponse&  response);
int	response_redirect(HttpResponse& response);
void    execute_cgi(HttpResponse &response);
void parse_query_string(HttpResponse &response);


#endif // WEBSERV

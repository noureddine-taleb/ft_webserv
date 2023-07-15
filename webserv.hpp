#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include "config.hpp"
#include "sched.hpp"
#include <algorithm>
#include <arpa/inet.h>
#include <cstdio>
#include <cstring>
#include <dirent.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>
#include <fcntl.h>

#define BACKLOG_SIZE 32
#define HTTP_DEL "\r\n"
#define HTTP_DEL_LEN (sizeof(HTTP_DEL) - 1)
#define BUFF_SIZE 4096 // 1

#define WATCHL_NO_PENDING -1
#define REQ_CONN_BROKEN -1
#define REQ_TO_BE_CONT -2
/////////////////////////////////////color//////////////////////////////////////////////
#define GRAY "\033[90m"
#define RED "\033[91m"
#define GREEN "\033[92m"
#define YELLOW "\033[93m"
#define BLUE "\033[94m"
#define PURPLE "\033[95m"
#define SKY "\033[96m"

#define END "\033[00m"

#define _debug(msg) std::cerr << msg
#define debug(msg) _debug("[debug] " << __FILE__ << ":" << __LINE__ << " " << msg << std::endl)

#define die(msg)                                                               \
  do {                                                                         \
    std::cerr << "[crit] " << msg << std::endl;                \
    exit(1);                                                                   \
  } while (0);

#define assert_msg(cond, msg)                                                      \
  do {                                                                         \
    if (!(cond))                                                               \
      die(msg);                                                                \
  } while (0);
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

class HttpRequest : public SchedulableEntity {
public:
  HttpRequest()
      : method(""), url(""), version(""), __http_top_header_parsed(false),
        __http_headers_end(false) {}
  std::string method;
  std::string url;
  std::string version;
  std::map<std::string, std::string> headers;
  std::vector<char> content;

  // derived from content
  std::vector<File> files;

  // pcb stuff
  std::vector<char> http_buffer;
  bool __http_top_header_parsed;
  bool __http_headers_end;
  enum SchedulableEntityTypes get_type() { return REQUEST; }
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
		std::vector<CGI>::iterator cgi_it;
		std::string    path_file;
		std::string old_url;
		bool get_length;
		int size_file;
		bool finish_reading;
		HttpRequest request;
		int byte_reading;
		int	fd;
		pid_t pid;
		std::string query_str;
		int	url_changed;
		std::vector<std::string> file_name_genarated;
		std::string cookies;
		int nbr_env;
		bool *close_connexion;
		// std::string name_output;
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
std::vector<std::string> split(std::string s, std::string delimiter,
									unsigned int max_splits = -1);
int ft_stoi(std::string str);
unsigned int ft_stoi_base_16(std::string str);

//----------------------------------------------------------------------------
//iterators
std::vector<Server>::iterator server(HttpRequest& request);
std::vector<Location>::iterator	location(HttpRequest& req, std::vector<Server>::iterator server);

//lib
int				ft_atoi(std::string s);
std::string 	get_content_type(std::string path);
std::string		type_repo(std::string path);
int				get_path(HttpResponse& response);
void			add_extention(std::string& filename,HttpResponse& response);
void 			dump_request(HttpRequest &request);
std::string		generate_filename(std::string &file, int *num);
void 			delete_generated_file(HttpResponse &response);

//read file
int     			read_File(HttpResponse& response);
std::string		read_File_error(std::string Path);
std::string		res_content_file(int status_code, HttpRequest& request, HttpResponse& response, std::string path);
std::string		ft_tostring(int nbr);

//read diractory
std::string		content_dir(std::string dir,HttpResponse& response, std::vector<std::string>& content);
int				res_content_dir(int status_code, HttpResponse& response);

//generate response
int 			new_request(HttpRequest &request, HttpResponse &response, int status_code);
void			init_response(HttpResponse& response, HttpRequest& request, int fd);
int				check_req_line_headers(HttpRequest &request);
std::string		res_content(int status_code, HttpResponse& response);
void			fill_response(int status_code, HttpResponse& response);
std::string		get_reason_phase(int status_code);
int 			response_Http_Request(int status_code , HttpResponse& response);
void			response_Http_Request(int status_code, HttpRequest& request, HttpResponse& response, std::string path);

//get response
void			response_Http_Request_error(int status_code, HttpResponse& response);
int				response_get(HttpResponse& response);
int       get_req(HttpResponse &response);

//post response
void			upload_exist(HttpResponse& response, std::string& upload_path);
int				upload_not_exist(HttpResponse& response);
int 			upload_not_exist_file(HttpResponse &response);
int       post_req(HttpResponse &response);
int 			response_post(HttpResponse& response);

// delete response
int 			response_delete(HttpResponse& response);

//redirection
int				response_redirect(HttpResponse& response);
int				response_rewrite(HttpResponse&  response);
int				response_redirect(HttpResponse& response);

//cgi
int    			execute_cgi(HttpResponse &response);
void 			parse_query_string(HttpResponse &response);
char** 			get_env(HttpResponse& response);
void 			check_extention(HttpResponse &response);
void 			cgi_response_content(HttpResponse & response, std::string &name_output);

//send
int				send_response(int fd, HttpRequest& request, HttpResponse& response, int status_code, bool *close_connexion);
void			ft_send_error(int status_code, HttpResponse& response);
int       check_connexion(int fd);



// int continue_previous_response(HttpResponse &response) ;
#endif // WEBSERV

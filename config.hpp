#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>
#include <vector>

class CGI {
	public:
		std::string file_extension;
		std::string cgi_pass;
};

class Redirection {
	public:
		std::string from;
		std::string to;
};

class Location {
	public:
		std::vector<std::string> methods;
		std::vector<Redirection> redirections;
		CGI cgi;
		std::string root;
		std::string index;
		bool list_dir_content;
};

class ErrorPage {
	public:
		int error_code;
		std::string page;
};

class Server {
	public:
		std::string ip;
		int port;
		std::vector<std::string> server_names;
		std::vector<Location> routes;
		std::vector<ErrorPage> error_pages;
		int __fd;
};

class Config {
	public:
		std::vector<Server> servers;
		std::vector<ErrorPage> default_error_pages;
};
#endif
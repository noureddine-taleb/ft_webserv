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

class Return {
	public:
		Return(): code(0), to("") {}
		int code;
		std::string to;
};

class Location {
	public:
		Location(): dir(""), index(""), autoindex(false), upload(false), target("/") {}
		std::vector<std::string> methods;
		std::vector<Redirection> redirections;
		std::vector<CGI> cgi;
		std::string dir;
		std::string index;
		bool autoindex;
		bool upload;
		std::string target;
		Return creturn;
};

class ErrorPage {
	public:
		ErrorPage(): error_code(-1), page("") {}
		int error_code;
		std::string page;
};
#define M (1024 * 1024)
class Server {
	public:
		Server(): ip("0.0.0.0"), port(0), root("") {}
		std::string ip;
		int port;
		std::string root;
		std::vector<std::string> server_names;
		std::vector<Location> routes;
		std::vector<ErrorPage> error_pages;
};

class Config {
	public:
		Config(): client_max_body_size(1 * M), max_server_fd(0) {}
		std::vector<Server> servers;
		std::vector<ErrorPage> default_error_pages;
		long client_max_body_size;
		int max_server_fd;
};
#endif
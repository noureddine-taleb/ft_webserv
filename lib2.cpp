#include "webserv.hpp"
#include "config.hpp"
#include <iostream>
// #include <fstream>
// #include <sstream>
// #include <sys/stat.h>

int ft_atoi(std::string s) {
	int					sign = 1;
	int				  /*frac = 0.1,*/ nbr = 0;
	std::string::iterator it;

	for (it = s.begin(); *it == ' '; ++it)
		;
	if ((*it == '+' || *it == '-') && isdigit(*(it + 1)))
	{
		if (*it == '-')
			sign = -1;
		it++;
	}
	else if (*it == '+' && !isdigit(*(it + 1)))
		return (-1);
	while (it != s.end() && isdigit(*it))
		nbr = nbr * 10 + (*(it++) - '0');
	return (sign * nbr);
}

std::string get_content_type(HttpRequest& req)
{
	std::map<std::string, std::string> content_type;
	std::string type = req.url.substr(req.url.rfind(".") + 1,req.url.length());

	content_type["html"] = "text/html";
	content_type["htm"] = "text/html";
	content_type["shtml"] = "text/html";
	content_type["css"] = "text/css";
	content_type["js"] = "text/javascript";
	content_type["png"] = "image/png";
	content_type["json"] = "application/json";
	content_type["xml"] = "application/xml";
	content_type["pdf"] = "application/pdf";
	content_type["xml"] = "application/xml";
	content_type["jpeg"] = "image/jpeg";
	content_type["jpg"] = "image/jpeg";
	content_type["gif"] = "image/gif";
	content_type["txt"] = "text/plain";
	content_type["mp4"] = "video/mp4";

	if (content_type.find(type) != content_type.end())
		return (content_type[type]);
	return("application/octet-stream");
}

std::vector<Server>::iterator server(Config& config, HttpRequest& request)
{
	int	position = request.headers["Host"].find(":");
	std::string ip = request.headers["Host"].substr(0, position);
	int port = ft_atoi(request.headers["Host"].substr(position + 1, request.headers["Host"].length()));
	for (std::vector<Server>::iterator it = config.servers.begin(); it != config.servers.end(); it++)
		if (it->ip == ip && it->port == port)
			return (it);
	return (config.servers.begin());
}

std::vector<Location>::iterator location(HttpRequest& req, std::vector<Server>::iterator server)
{
	for (std::vector<Location>::iterator it2 = server->routes.begin(); it2 != server->routes.end();it2++)
		if (req.url.find(it2->target) != std::string::npos && req.url.find(it2->target) == 0)
			return (it2);
	return (server->routes.end());
}

std::string read_File(HttpResponse& response)
{
	std::ifstream file;
	file.open(response.path_file, std::ifstream::binary);

	if (file.is_open())
	{
		file.seekg (0, file.end);
		int length = file.tellg();
		file.seekg (0, file.beg);
		std::vector<char> buffer(length);

		if (response.get_length == false)
		{
			response.get_length = true;
			return (std::to_string(length));
		}
		if (response.byte_reading < length)
		{
			// int chunkSize = std::min(BUFF_SIZE, length - byte_reading);
			int chunkSize = std::min(length, length - response.byte_reading);
			buffer.resize(chunkSize);
			file.seekg(response.position);
			file.read(buffer.data(), chunkSize);
			response.position = file.tellg();
			response.byte_reading += file.gcount();
			if (file.gcount() == length)
			{
				response.finish_reading = true;
				response.position = 0;
				file.close();
			}
			// if (file.gcount() == 0)
			// {
			// file.close();
			// 	return ("finish_reading");
			// }
		}
		std::string content(buffer.begin(), buffer.end());
		return (content);
	}
	return ("404");
}

std::string read_File_error(std::string Path)
{
	std::ifstream file(Path);
	std::stringstream buffer;

	if (!file)
		return ("not found");
	buffer << file.rdbuf();
	return buffer.str();
}

std::string type_repo(std::string path)
{
	struct stat info;

	if (*(path.end() - 1) == '/')
		return ("is_directory with /");
	if (!stat(path.c_str(), &info))
	{
		if (S_ISREG(info.st_mode))
			return ("is_file");
		if (S_ISDIR(info.st_mode))
			return ("is_directory");
	}
	return ("not found");
}

std::string content_dir(std::string dir, std::vector<std::string>& content)
{
	DIR* directory = opendir(dir.c_str());

	if (directory)
	{
		struct dirent* content_dir;
        while ((content_dir = readdir(directory)))
			content.push_back(content_dir->d_name);
        closedir(directory);
		return("found");
	}
	return ("not found");
}

void	ft_send_error(int status_code, Config config, HttpResponse& response)
{
	std::string		response_buffer;

	response_Http_Request_error(status_code, config, response);
	response_buffer = generate_http_response(response);
	response_buffer += response.content;
	send(response.fd, response_buffer.c_str(), response_buffer.length(), 0);
}

void init_response(Config& config, HttpResponse& response, HttpRequest& request, int fd)
{
	response.fd = fd;
	response.byte_reading = 0;
	response.request = request;
	response.get_length = false;
	response.finish_reading = false;
	response.server_it = server(config, response.request);
	response.location_it = location(response.request, response.server_it);
}

void fill_response(int status_code, HttpResponse& response)
{
	response.version = response.request.version;
	response.code = status_code;
	response.reason_phrase = get_reason_phase(status_code);
	response.headers["Connection"] = "keep-alive";
	response.headers["Content-Type"] = get_content_type(response.request);
}

void get_path(HttpResponse& response)
{
// 	std::cout << "*********************** dir = " << response.location_it->dir << std::endl;
// 	std::cout << "*********************** target = " << response.location_it->target << std::endl;
// 	std::cout << "*********************** url = " << response.request.url << std::endl;
// 	std::cout << "*********************** " << response.request.url.substr(response.location_it->target.length(), response.request.url.length()) << std::endl;
	if (!response.location_it->dir.empty())
		response.path_file = response.location_it->dir + response.request.url.substr(response.location_it->target.length(), response.request.url.length());
	else if (!response.server_it->root.empty())
		response.path_file = response.server_it->root + response.request.url.substr(response.location_it->target.length(), response.request.url.length());
}

std::string	get_reason_phase(int status_code)
{
	std::map<int, std::string> reason_phase;

	reason_phase[301] = "Moved Permanently"; 
	reason_phase[400] = "Bad Request";
	reason_phase[403] = "403 Forbidden";
	reason_phase[404] = "Not Found";
	reason_phase[405] = "Method Not Allowed";
	reason_phase[413] = "Request Entity Too Large";
	reason_phase[414] = "Request-URI Too Long";
	reason_phase[501] = "not implemented";
	reason_phase[200] = "ok";

	return(reason_phase[status_code]);
}

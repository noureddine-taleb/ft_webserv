#include "webserv.hpp"
#include "config.hpp"
#include <iostream>

int ft_atoi(std::string s) {
	int					sign = 1;
	int					nbr = 0;
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

std::string get_content_type(std::string path)
{
	std::map<std::string, std::string> content_type;
	std::string type = path.substr(path.rfind(".") + 1,path.length());

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

std::vector<Location>::iterator location(Config& config, HttpRequest& req, std::vector<Server>::iterator server)
{
	unsigned long	length_location(0);
	std::vector<Location>::iterator location = server->routes.end();
	std::vector<Redirection>::iterator redirection_it;

	(void)config;
	for (std::vector<Location>::iterator location_it = server->routes.begin(); location_it != server->routes.end();location_it++)
	{
		if (req.url.find(location_it->target) != std::string::npos)
		{
			if ((location_it->target.length()) > length_location)
			{
				length_location = location_it->target.length();
				location = location_it;
			}
		}
	}
	if (location != server->routes.end() && !location->redirections.empty())
	{
		for (redirection_it = location->redirections.begin(); redirection_it != location->redirections.end(); redirection_it++)
		{
			size_t find_from = req.url.find(redirection_it->from);
			size_t find_to = req.url.find(redirection_it->to);

			if (find_from != std::string::npos && find_to == std::string::npos)
			{
				req.url = req.url.substr(0,find_from) + redirection_it->to + req.url.substr((find_from + redirection_it->from.length()),req.url.length());
				break;
			}
		}
	}
	return (location);
}

std::string read_File(HttpResponse& response)
{

	std::ifstream file;
	std::string res = "";
	std::stringstream hex;
	file.open(response.path_file, std::ifstream::binary);

	if (file.is_open())
	{
		file.seekg (0, file.end);
		int length = file.tellg();
		file.seekg (0, file.beg);
		std::vector<char> buffer(BUFF_SIZE);

		if (response.get_length == false)
		{
			response.get_length = true;
			return (ft_tostring(length));
		}
		if (response.byte_reading < length)
		{
			// int chunkSize = std::min(BUFF_SIZE, length - response.byte_reading);
			int chunkSize = std::min(length, length - response.byte_reading);
			hex << std::hex << chunkSize;
			buffer.resize(chunkSize);
			res += hex.str() + "\r\n";
			file.seekg(response.byte_reading);
			file.read(buffer.data(), chunkSize);
			response.byte_reading += file.gcount();

			std::string content(buffer.begin(), buffer.end());
			res += content + HTTP_DEL;
			if (file.gcount() == length)
			{
				hex.str("");
				chunkSize = 0;
				hex << std::hex << chunkSize;
				response.finish_reading = true;
				res += hex.str() + HTTP_DEL + HTTP_DEL;
				file.close();
			}
			// if (file.gcount() == 0)
			// {
				// hex.str("");
				// chunkSize = 0;
				// hex << std::hex << chunkSize;
				// response.finish_reading = true;
				// res += hex.str() + HTTP_DEL + HTTP_DEL;
				// file.close();
			// }
		}
		return (res);
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
		return ("is_directory");
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
	response.location_it = location(config, response.request, response.server_it);
}

void fill_response(int status_code, HttpResponse& response)
{
	response.version = response.request.version;
	response.code = status_code;
	response.reason_phrase = get_reason_phase(status_code);
	response.headers["Connection"] = "keep-alive";
	response.headers["Content-Type"] = get_content_type(response.path_file);
}

int get_path(Config config, HttpResponse& response)
{
	if (!response.location_it->dir.empty())
	{
		response.path_file = response.location_it->dir + response.request.url.substr(response.location_it->target.length(), response.request.url.length());
		return (1);
	}
	if (!response.server_it->root.empty())
	{
		response.path_file = response.server_it->root + response.request.url.substr(response.location_it->target.length(), response.request.url.length());
		return (1);
	}
	ft_send_error(404, config, response);
	return (0);
}

std::string	get_reason_phase(int status_code)
{
	std::map<int, std::string> reason_phase;

	reason_phase[301] = "Moved Permanently"; 
	reason_phase[302] = "Found"; 
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

std::string	ft_tostring(int nbr)
{
	std::string	str;

	while (nbr > 10)
	{
		str.insert(0,1, static_cast<char>((nbr % 10) + '0'));
		nbr /= 10;
	}
	if (nbr > 0)
		str.insert(0,1, static_cast<char>(nbr+ '0'));
	return (str);
}

int	response_redirect(HttpResponse& response, Config& config)
{
	std::string type_rep;
	std::string	response_buffer;

	response.path_file = response.location_it->creturn.to;
	type_rep = type_repo(response.path_file);
	if (type_rep == "is_file")
	{
		if (response.location_it->cgi.empty())
		{
			response_Http_Request(200, config, response);
			return (1);
		}
	}
	else if (type_rep == "is_directory")
	{
		if (response_Http_Request(301,config, response))
			return (1);
	}
	else
	{
		if (response.location_it->creturn.code)
			fill_response(response.location_it->creturn.code, response);
		else
			fill_response(302, response);
		response.headers["location"] = response.location_it->creturn.to;
		response_buffer = generate_http_response(response);
		send(response.fd, response_buffer.c_str(), response_buffer.size(), 0);
	}
	return (0);
}

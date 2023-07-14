#include "../webserv.hpp"
#include "../config.hpp"
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

std::string	ft_tostring(int nbr)
{
	std::string	str;

	while (nbr >= 10)
	{
		str.insert(0,1, static_cast<char>((nbr % 10) + '0'));
		nbr /= 10;
	}
	if (nbr >= 0)
		str.insert(0,1, static_cast<char>(nbr+ '0'));
	return (str);
}

std::vector<Server>::iterator server(HttpRequest& request)
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
	unsigned long	length_location(0);
	std::vector<Location>::iterator location = server->routes.end();
	std::vector<Redirection>::iterator redirection_it;

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
			if (get_content_type(req.url) == "application/octet-stream")
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
	}
	return (location);
}

void	ft_send_error(int status_code, HttpResponse& response)
{
	std::string		response_buffer;
	
	response_Http_Request_error(status_code, response);
	response_buffer = generate_http_response(response);
	response_buffer += response.content_error;
	int ret = send(response.fd, response_buffer.c_str(), response_buffer.length(), 0);
	if (ret < 0)
	{
		perror("send feiled");
		// *response.close_connexion = true;
	}
}

void check_extention(HttpResponse &response)
{
	std::string path = response.path_file;
	std::vector<CGI>::iterator cgi_it;

	response.cgi_it = response.location_it->cgi.end();
	for (cgi_it = response.location_it->cgi.begin(); cgi_it != response.location_it->cgi.end(); cgi_it++)
	{
		if (path.substr(path.find_last_of(".") + 1, path.length()) == cgi_it->file_extension
			&& (cgi_it->file_extension == "php" || cgi_it->file_extension == "py"))
		{
			response.cgi_it = cgi_it;
			break ;
		}
	}
}

void delete_generated_file(HttpResponse &response)
{
	std::vector<std::string>::iterator files_it;

	for(files_it = response.file_name_genarated.begin(); files_it != response.file_name_genarated.end(); files_it++)
		unlink((*files_it).c_str());
}
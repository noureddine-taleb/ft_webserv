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

std::string	ft_tostring(int nbr)
{
	std::string	str;

	while (nbr > 10)
	{
		str.insert(0,1, static_cast<char>((nbr % 10) + '0'));
		nbr /= 10;
	}
	if (nbr >= 0)
		str.insert(0,1, static_cast<char>(nbr+ '0'));
	return (str);
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

void	ft_send_error(int status_code, Config config, HttpResponse& response)
{
	std::string		response_buffer;

	response_Http_Request_error(status_code, config, response);
	response_buffer = generate_http_response(response);
	response_buffer += response.content;
	send(response.fd, response_buffer.c_str(), response_buffer.length(), 0);
}

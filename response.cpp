#include "webserv.hpp"
#include "config.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <dirent.h>

std::string generate_http_response(HttpResponse &res)
{
	std::stringstream res_str;
	res_str << res.version << " " << res.code<< " " << res.reason_phrase << HTTP_DEL;
	for (std::map<std::string, std::string>::iterator it = res.headers.begin(); it != res.headers.end(); it++)
		res_str << it->first << ": " << it->second << HTTP_DEL;
	res_str << HTTP_DEL;
	// res_str << res.content;
	return res_str.str();
}

std::string		res_content(int status_code, Config& config, HttpResponse& response)
{
	// std::vector<Server>::iterator server_it = server(config, request);
	
	for (std::vector<ErrorPage>::iterator it = response.server_it->error_pages.begin(); it != response.server_it->error_pages.end(); it++)
	{
		if (it->error_code == status_code)
			return(read_File_error(it->page));
	}
	for (std::vector<ErrorPage>::iterator it = config.default_error_pages.begin(); it != config.default_error_pages.end(); it++)
	{		
		if (it->error_code == status_code)
			return (read_File_error(it->page));
	}
	return ("not found");
}

int response_Http_Request(int status_code , Config& config, HttpResponse& response)
{
	// response.version = response.request.version;
	// response.code = status_code;
	fill_response(status_code, response);
	switch (status_code)
	{
		case 301:
			response.reason_phrase = "Moved Permanently";
			if (res_content_dir(status_code, config, response))
				return (1);
			// response.headers["Content-Type"] = get_content_type(response.request);
			// response.headers["location"] = "https://profile.intra.42.fr/";
			break;
		case 200:
			response.reason_phrase = "ok";
			return (1);
			// response.headers["Content-Type"] = get_content_type(response.request);
	}
	return (0);
}

int	response_get(Config& config, HttpResponse& response)
{
		std::string type_rep;
		get_path(response);
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
			ft_send_error(404, config, response);
return (0);
}

int	res_content_dir(int status_code, Config& config, HttpResponse& response)
{
	std::vector<std::string>			content;
	std::vector<std::string>::iterator	content_it;
	std::string							response_buffer;
	std::string::iterator				url_it = response.request.url.end();
	
	(void) status_code;
	if (content_dir(response.path_file, content) == "found")
	{
		if (!response.location_it->index.empty())
		{
			content_it = std::find(content.begin(), content.end(), response.location_it->index);
			if (content_it != content.end())
			{
				if (*(url_it - 1) != '/')
					response.request.url += "/" + response.location_it->index;
				else
				{
					response.code = 200;
					response.reason_phrase = "ok";
					response.request.url += response.location_it->index;
				}
				get_path(response);
				response.headers["Content-Type"] = get_content_type(response.request);
				return(1) ;
			}
		}
		content_it = std::find(content.begin(), content.end(), "index.html");
		if (content_it != content.end())
		{
			if (*(url_it - 1) != '/')// rbegin()
				response.request.url += "/index.html";
			else
			{
				response.code = 200;
				response.reason_phrase = "ok";
				response.request.url += "index.html";
			}
			get_path(response);
			response.headers["Content-Type"] = get_content_type(response.request);
			return(1) ;
		}
		if (response.location_it->autoindex)
		{
			for (std::vector<std::string>::iterator it = content.begin(); it != content.end(); it++)
				response.content += *it + "\n";
			response.headers["Content-Length"] = std::to_string(response.content.length());
			response_buffer = generate_http_response(response);
			send(response.fd, response_buffer.c_str(), response_buffer.length(), 0) ;
			return (0);
		}
		else
		{
			ft_send_error(403, config, response);
			return(0);
		}
	}
	ft_send_error(403, config, response);
	return(0);
}

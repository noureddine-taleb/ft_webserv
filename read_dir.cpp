#include "webserv.hpp"
#include "config.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <dirent.h>

std::string content_dir(std::string dir,HttpResponse& response, std::vector<std::string>& content)
{
	DIR* directory = opendir(dir.c_str());

	if (directory)
	{
		struct dirent* content_dir;
		std::ofstream file("content_dir.html");
		if (file)
		{
			if (response.request.method == "GET")
			{
				file << "<!DOCTYPE html>\n" << "<html>\n" << "<head>\n" << "<title>autoindex</title>\n";
				file << "</head>\n" << "<body>\n";
				file << "<h1>" << "Index of " << response.request.url <<"</h1>\n" << "<ul>\n";
			}
			while ((content_dir = readdir(directory)))
			{
				if (strcmp(content_dir->d_name, ".") && strcmp(content_dir->d_name , "..")
						&& strcmp(content_dir->d_name, ".DS_Store"))
				{
					if (response.request.method == "GET")
					{
						if (*response.request.url.rbegin() != '/')
							file << "<li><a href=\"" << "http://"<< response.server_it->ip << ":" << response.server_it->port << response.request.url << '/' << content_dir->d_name << "\">" << content_dir->d_name << "</a></li>\n";
						else
							file << "<li><a href=\"" << "http://"<< response.server_it->ip << ":" << response.server_it->port << response.request.url << content_dir->d_name << "\">" << content_dir->d_name << "</a></li>\n";
					}
					content.push_back(content_dir->d_name);
				}
			}
			if (response.request.method == "GET")
				file << "</ul>\n" << "</body>\n" << "</html>\n";
		}
		file.close();
		closedir(directory);
		return("found");
	}
	return ("not found");
}

int	res_content_dir(int status_code, Config& config, HttpResponse& response)
{
	std::vector<std::string>			content;
	std::vector<std::string>::iterator	content_it;
	std::string							response_buffer;
	
	(void) status_code;
	if (content_dir(response.path_file, response, content) == "found")
	{
		if (!response.location_it->index.empty())
		{
			content_it = std::find(content.begin(), content.end(), response.location_it->index);
			if (content_it != content.end())
			{
				if (*response.path_file.rbegin() != '/')
					response.path_file += "/" + response.location_it->index;
				else
				{
					response.code = 200;
					response.reason_phrase = "ok";
					response.path_file += response.location_it->index;
				}
				response.headers["Content-Type"] = get_content_type(response.path_file);
				return(1) ;
			}
		}
		content_it = std::find(content.begin(), content.end(), "index.html");
		if (content_it != content.end())
		{
			if (*response.path_file.rbegin() != '/')
				response.path_file += "/index.html";
			else
			{
				response.code = 200;
				response.reason_phrase = "ok";
				response.path_file += "index.html";
			}
			response.headers["Content-Type"] = get_content_type(response.path_file);
			return(1) ;
		}
		if (response.location_it->autoindex && response.request.method == "GET")
		{
			response.path_file = "content_dir.html";
			fill_response(200, response);
			return (1);
		}
		else
		{
			ft_send_error(403, config, response);
			return(0);
		}
	}
	ft_send_error(404, config, response);
	return(0);
}

#include "../webserv.hpp"
#include "../config.hpp"

int	response_get(HttpResponse& response)
{
	std::string type_rep;
	std::string path = response.path_file;

	if (!response.location_it->creturn.to.empty())
		return (response_redirect(response));
	
	if (get_path(response))
	{
		type_rep = type_repo(response.path_file);
		if (type_rep == "is_file")
		{
			if (response.location_it->cgi.empty())
			{
				if(response_Http_Request(200, response))
					return (1);
			}
			if(!response.location_it->cgi.empty())
			{
				check_extention(response);
				if (!response.location_it->cgi.empty() && response.cgi_it == response.location_it->cgi.end())
				{
					if(response_Http_Request(200, response))
						return (1);
				}
				else
				{
					fill_response(200, response);
					return (execute_cgi(response));
				}
			}
		}
		else if (type_rep == "is_directory")
		{
			if (*response.request.url.rbegin() != '/')
			{
				fill_response(301, response);
				response.headers["Location"] = response.request.url + "/";
				response.url_changed = true;
				std::string response_buffer = generate_http_response(response);
				if (check_connexion(response.fd) < 0)
					return (-1);
				int ret = send(response.fd, response_buffer.c_str(), response_buffer.size(), 0);
				if (ret == 0)
					return (-1);
				if (ret < 0)
					return (0);
				response.finish_reading = true;
				*response.close_connexion = true;
				return (0);
			}
			else if (response_Http_Request(301, response))
				return (1);
		}
		else
		{
			*response.close_connexion = true;
			ft_send_error(404, response);
		}
	}
	return (0);
}

int get_req(HttpResponse &response)
{
	std::string response_buffer;
	std::string content_length;
	std::string content;
	int i(0);

	if (response_get(response) )
	{
		read_File(response);
		content_length = ft_tostring(response.size_file);
		if (response.size_file < (BUFF_SIZE / 10))
		{
			content = read_File_error(response.path_file, response);
			*response.close_connexion = true;
			i = 1;
		}
		response.headers["content-length"] = content_length;
		response_buffer = generate_http_response(response);
		response_buffer += content;
		if (check_connexion(response.fd) < 0)
			return (-1);
		int ret = send(response.fd, response_buffer.c_str(), response_buffer.length(), 0);
		if (ret == 0)
		{
			*response.close_connexion = true;
			return (-1);
		}
		if (ret < 0)
			return (1);
		if (i)
			return (1);
	}
	else
	{
		if (!response.is_loop)
			*response.close_connexion = true;
		return (1);
	}
	return (0);
}


#include "../webserv.hpp"
#include "../config.hpp"

std::string		res_content(int status_code, HttpResponse& response)
{
	for (std::vector<ErrorPage>::iterator it = response.server_it->error_pages.begin(); it != response.server_it->error_pages.end(); it++)
	{
		if (it->error_code == status_code)
		{
			response.headers["Content-type"] = get_content_type(it->page);
			return(read_File_error(it->page));
		}
	}
	for (std::vector<ErrorPage>::iterator it = config.default_error_pages.begin(); it != config.default_error_pages.end(); it++)
	{		
		if (it->error_code == status_code)
		{
			response.headers["Content-type"] = get_content_type(it->page);
			return (read_File_error(it->page));
		}
	}
	return ("not found");
}

int response_Http_Request(int status_code , HttpResponse& response)
{
	if (!response.url_changed)
		fill_response(status_code, response);
	else
	{
		if (status_code != 301 && status_code != 302)
			fill_response(302, response);
		else
			fill_response(status_code, response);
		return (response_rewrite(response));
	}
	switch (status_code)
	{
		case 301:
			if (res_content_dir(status_code, response))
				return (1);
			break;
		case 200:
			return (1);
	}
	return (0);
}

int	response_get(HttpResponse& response)
{
	std::string type_rep;

	if (!response.location_it->creturn.to.empty())
		return (response_redirect(response));
	
	if (get_path(response))
	{
		type_rep = type_repo(response.path_file);
		if (type_rep == "is_file")
		{
			check_extention(response);
			if ((!response.location_it->cgi.empty() && response.cgi_it == response.location_it->cgi.end())
				|| response.location_it->cgi.empty())
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
		else if (type_rep == "is_directory")
		{
			if (*response.path_file.rbegin() != '/')
			{
				fill_response(301, response);
				response.headers["Location"] = response.request.url + "/";
				response.url_changed = true;
				std::string response_buffer = generate_http_response(response);
				int ret = send(response.fd, response_buffer.c_str(), response_buffer.size(), 0);
				if (ret < 0)
				{
					perror("send feiled");
					// *response.close_connexion = true;
				}
				return (0);
			}
			else if (response_Http_Request(301, response))
				return (1);
		}
		else
			ft_send_error(404, response);
	}
	return (0);
}


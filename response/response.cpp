
#include "../webserv.hpp"
#include "../config.hpp"

std::string		res_content(int status_code, HttpResponse& response)
{
	for (std::vector<ErrorPage>::iterator it = response.server_it->error_pages.begin(); it != response.server_it->error_pages.end(); it++)
	{
		if (it->error_code == status_code)
		{
			response.headers["Content-Type"] = get_content_type(it->page);
			return(read_File_error(it->page));
		}
	}
	for (std::vector<ErrorPage>::iterator it = config.default_error_pages.begin(); it != config.default_error_pages.end(); it++)
	{		
		if (it->error_code == status_code)
		{
			response.headers["Content-Type"] = get_content_type(it->page);
			return (read_File_error(it->page));
		}
	}
	return ("not found");
}
// int	response_rewrite(HttpResponse&  response)
// {
// 	std::cout << "\033[33m" << "////////////////////////> "<< response.request.url << "\033[0m" << std::endl;
// 	response.headers["Location"] = response.request.url;
// 	std::string response_buffer = generate_http_response(response);
// 	// response_buffer += response.content;
// 	send(response.fd, response_buffer.c_str(), response_buffer.size(), 0);
// 	return (0);
// }

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

std::vector<CGI>::iterator check_extention(HttpResponse &response)
{
	std::string path = response.path_file;
	std::vector<CGI>::iterator cgi_it;

	if (!response.location_it->cgi.empty())
	{
		for (cgi_it = response.location_it->cgi.begin(); cgi_it != response.location_it->cgi.end(); cgi_it++)
		{
			if (path.substr(path.find_last_of(".") + 1, path.length()) == cgi_it->file_extension)
				break ;
		}
	}
	return (cgi_it);
}

int	response_get(HttpResponse& response)
{
	std::string type_rep;

	if (!response.location_it->creturn.to.empty())
		return (response_redirect(response));
	if (get_path(response))
	{
		// std::cout << "\033[32m path == " << response.query_str << "\033[00m" << std::endl;
		// if (!response.query_str.empty())
		// 	parse_query_string(response);
		std::cout << "path == " << response.path_file << std::endl;
		type_rep = type_repo(response.path_file);
		if (type_rep == "is_file")
		{

			if (check_extention(response) == response.location_it->cgi.end())
			{
				if(response_Http_Request(200, response))
					return (1);
			}
			else
			{
				// std::cout << "!!!!!!!!!!!!!!! = " <<type_rep<< std::endl;
				fill_response(200, response);
				execute_cgi(response);
				return (1);
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
				send(response.fd, response_buffer.c_str(), response_buffer.size(), 0);
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

// int	response_redirect(HttpResponse& response, Config& config)
// {
// 	std::string type_rep;
// 	std::string	response_buffer;

// 	response.path_file = response.location_it->creturn.to;
// 	response.request.url = response.path_file ;
// 	type_rep = type_repo(response.path_file);
// 	if (type_rep == "is_file")
// 	{
// 		if (response.location_it->cgi.empty())
// 		{
// 			response_Http_Request(200, config, response);
// 			return (1);
// 		}
// 	}
// 	else if (type_rep == "is_directory")
// 	{
// 		if (response_Http_Request(301,config, response))
// 			return (1);
// 	}
// 	else
// 	{
// 		if (response.location_it->creturn.code)
// 			fill_response(response.location_it->creturn.code, response);
// 		else
// 			fill_response(302, response);
// 		response.headers["location"] = response.location_it->creturn.to;
// 		response_buffer = generate_http_response(response);
// 		send(response.fd, response_buffer.c_str(), response_buffer.size(), 0);
// 	};
// 	return (0);
// }


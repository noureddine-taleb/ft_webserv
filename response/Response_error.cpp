#include "../webserv.hpp"
#include "../config.hpp"

int	check_req_line_headers(HttpRequest &request, HttpResponse &response)
{
	
	std::vector<Server>::iterator server_it = server(request);
	std::vector<Location>::iterator location_it = location(request, server_it, response);
	std::vector<std::string>::iterator methods_it;

	if(location_it != server_it->routes.end())
	{
		for (methods_it = location_it->methods.begin(); methods_it != location_it->methods.end(); methods_it++)
		{
			if(*methods_it == request.method)
			{
				if (request.method == "GET")
					return(1);
				if (request.method == "POST")
					return(2);
				if (request.method == "DELETE")
					return(3);
				break;
			}
		}
		if (methods_it == location_it->methods.end())
			return (405);
	}
	return (404);
}

void get_content_error(HttpResponse &response, int status_code,const std::string &path)
{
	response.content_error = res_content(status_code, response);
	if (response.content_error == "not found")
	{
		response.headers["Content-type"] = get_content_type(path);
		response.content_error = read_File_error(path, response);
	}
}

void response_Http_Request_error(int status_code, HttpResponse& response)
{
	fill_response(status_code, response);
	switch(status_code) {
		case 501 :
			get_content_error(response, status_code , std::string("www/501.html"));
			break ;
		case 500 :
			get_content_error(response, status_code , std::string("www/500.html"));
			break ;
		case 400:
			get_content_error(response, status_code , std::string("www/400.html"));
			break ;
		case 414:
			get_content_error(response, status_code , std::string("www/414.html"));
			break ;
		case 413:
			get_content_error(response, status_code , std::string("www/413.html"));
			break ;
		case 404:
			get_content_error(response, status_code , std::string("www/404.html"));
			break ;
		case 405:
			get_content_error(response, status_code , std::string("www/405.html"));
			break;
		case 403:
			get_content_error(response, status_code , std::string("www/403.html"));
			break ;
		case 204:
			get_content_error(response, status_code , std::string("www/204.html"));
			break ;
		case 409:
			get_content_error(response, status_code , std::string("www/409.html"));
		case 504:
			get_content_error(response, status_code , std::string("www/504.html"));
		case 508:
			get_content_error(response, status_code , std::string("www/508.html"));
	}
	response.headers["Content-Length"] = ft_tostring(response.content_error.length());
}

void	ft_send_error(int status_code, HttpResponse& response)
{
	std::string		response_buffer;
	
	*response.close_connexion = true;
	response_Http_Request_error(status_code, response);
	response_buffer = generate_http_response(response);
	response_buffer += response.content_error;
	if (check_connexion(response.fd) < 0)
	{
		*response.close_connexion = true;
		return ;
	}
	int ret = send(response.fd, response_buffer.c_str(), response_buffer.length(), 0);
	if (ret == 0)
		*response.close_connexion = true;
	else if (ret < 0)
	{
		*response.close_connexion = false;
		response.finish_reading = false;
	}
	return ;

}
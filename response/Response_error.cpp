#include "../webserv.hpp"
#include "../config.hpp"
#include <iostream>
#include <fstream>
#include <sstream>


int	check_req_line_headers(HttpRequest &request)
{
	
	std::vector<Server>::iterator server_it = server(config, request);
	std::vector<Location>::iterator location_it = location(request, server_it);
	std::vector<std::string>::iterator methods_it;
// 	// std::map<std::string, std::string>::iterator header_it = request.headers.find("Transfer-Encoding");
// 	// std::string client_max_body_size = "100M";//the maximum size limit in megabytes (MB) for the request body.
// 	// std::string client_max_body = client_max_body_size.substr(0, client_max_body_size.find("M"));//10

// 	// if (header_it != request.headers.end() && header_it->second.erase(0, 1) != "chunked")
// 	// 	return (501);//not implemented
// 	// if (header_it == request.headers.end()
// 	// 	&& request.headers.find("Content-Length") == request.headers.end() && request.method == "POST")
// 	// 	return(400);//Bad Request
// 	// if (request.url.find_first_not_of("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~:/?#[]@!$&'()*+,;=%")
// 	// 		!= std::string::npos)
// 	// 	return(400);//Bad Request
// 	// if (request.url.length() > 2048)
// 	// 	return (414);//Request-URI Too Long
// 	// // if (ft_atoi(request.headers["Content-Length"]) > ft_atoi(client_max_body) * 1024 * 1024)
// 	// // 	return (413);
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

void response_Http_Request_error(int status_code, Config& config, HttpResponse& response)
{
	fill_response(status_code, response);
	switch(status_code) {
		case 501 :
			response.content_error = res_content(status_code, config, response);
			if (response.content_error == "not found")
				response.content_error = read_File_error("www/501.html");
			break ;
		case 400:
			response.content_error = res_content(status_code, config, response);
			if (response.content_error == "not found")
				response.content_error = read_File_error("www/400.html");
			break ;
		case 414:
			response.content_error = res_content(status_code, config, response);
			if (response.content_error == "not found")
				response.content_error = read_File_error("www/414.html");
			break ;
		case 413:
			response.content_error = res_content(status_code, config, response);
			if (response.content_error == "not found")
				response.content_error = read_File_error("www/413.html");
			break ;
		case 404:
			response.content_error = res_content(status_code, config, response);
			if (response.content_error == "not found")
				response.content_error = read_File_error("www/404.html");
			break ;
		case 405:
			response.content_error = res_content(status_code, config, response);
			if (response.content_error == "not found")
				response.content_error = read_File_error("www/405.html");
			break;
		case 403:
			response.content_error = res_content(status_code, config, response);
			if (response.content_error == "not found")
				response.content_error = read_File_error("www/403.html");
			break ;
		case 204:
			response.content_error = res_content(status_code, config, response);
			if (response.content_error == "not found")
				response.content_error = read_File_error("www/204.html");
			break ;
		case 409:
			response.content_error = res_content(status_code, config, response);
			if (response.content_error == "not found")
				response.content_error = read_File_error("www/409.html");
	}
	response.headers["Content-Length"] = ft_tostring(response.content_error.length());
}


#include "webserv.hpp"
#include "config.hpp"
#include <iostream>
#include <fstream>
#include <sstream>


int	check_req_line_headers(Config& config, HttpRequest &request)
{
	std::vector<Server>::iterator server_it = server(config, request);
	std::vector<Location>::iterator location_it = location(request, server_it);
	std::map<std::string, std::string>::iterator header_it = request.headers.find("Transfer-Encoding");
	std::vector<std::string>::iterator methods_it;
	std::string client_max_body_size = "100M";//the maximum size limit in megabytes (MB) for the request body.
	std::string client_max_body = client_max_body_size.substr(0, client_max_body_size.find("M"));//10
	if (header_it != request.headers.end() && header_it->second.erase(0, 1) != "chunked")
		return (501);//not implemented
	if (header_it == request.headers.end()
		&& request.headers.find("Content-Length") == request.headers.end() && request.method == "POST")
		return(400);//Bad Request
	if (request.url.find_first_not_of("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~:/?#[]@!$&'()*+,;=%")
			!= std::string::npos)
		return(400);//Bad Request
	if (request.url.length() > 2048)
		return (414);//Request-URI Too Long
	// if (ft_atoi(request.headers["Content-Length"]) > ft_atoi(client_max_body) * 1024 * 1024)
	// 	return (413);
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
	response.version = response.request.version;
	response.code = status_code;
	switch(status_code) {
		case 501 :
			response.reason_phrase = "not implemented";
			response.content = res_content(status_code, config, response);
			if (response.content == "not found")
				response.content = read_File_error("www/501.html");
			response.headers["Content-Type"] = get_content_type(response.request);
			break ;
		case 400:
			response.reason_phrase = "Bad Request";
			response.content = res_content(status_code, config, response);
			if (response.content == "not found")
				response.content = read_File_error("www/400.html");
			response.headers["Content-Type"] = get_content_type(response.request);
			break ;
		case 414:
			response.reason_phrase = "Request-URI Too Long";
			response.content = res_content(status_code, config, response);
			if (response.content == "not found")
				response.content = read_File_error("www/414.html");
			response.headers["Content-Type"] = get_content_type(response.request);
			break ;
		case 413:
			response.reason_phrase = "Request Entity Too Large";
			response.content = res_content(status_code, config, response);
			if (response.content == "not found")
				response.content = read_File_error("www/413.html");
			response.headers["Content-Type"] = get_content_type(response.request);
			break ;
		case 404:
			response.reason_phrase = "Not Found";
			response.content = res_content(status_code, config, response);
			if (response.content == "not found")
				response.content = read_File_error("www/404.html");
			response.headers["Content-Type"] = get_content_type(response.request);
			break ;
		case 405:
			response.reason_phrase = "Method Not Allowed";
			response.content = res_content(status_code, config, response);
			if (response.content == "not found")
				response.content = read_File_error("www/405.html");
			response.headers["Content-Type"] = get_content_type(response.request);
			break;
		case 403:
				response.reason_phrase = "403 Forbidden";
			response.content = res_content(status_code, config, response);
			if (response.content == "not found")
				response.content = read_File_error("www/403.html");
			response.headers["Content-Type"] = get_content_type(response.request);
	}
	response.headers["Content-Length"] = std::to_string(response.content.length());
}


// 0.0.0.0:8080/srcs/
//srckjbfk in uri ^













// int	check_req_well_formed(int fd,Config& config, std::map<int,HttpResponse>& responses)
// {
// 	std::map<std::string, std::string>::iterator it;
// 	std::vector<std::string>::iterator it4;
// 	std::string client_max_body_size = "100M";//the maximum size limit in megabytes (MB) for the request body.
// 	std::string client_max_body = client_max_body_size.substr(0, client_max_body_size.find("M"));//10

// 	it = responses[fd].request.headers.find("Transfer-Encoding");
// 	if (it != responses[fd].request.headers.end() && it->second.erase(0, 1) != "chunked")
// 		return (501);//not implemented
// 	if (it == responses[fd].request.headers.end()
// 		&& responses[fd].request.headers.find("Content-Length") == responses[fd].request.headers.end() && responses[fd].request.method == "POST")
// 		return(400);//Bad Request
// 	if (responses[fd].request.url.find_first_not_of("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~:/?#[]@!$&'()*+,;=%")
// 			!= std::string::npos)
// 		return(400);//Bad Request
// 	if (responses[fd].request.url.length() > 2048)
// 		return (414);//Request-URI Too Long
// 	if (ft_atoi(responses[fd].request.headers["Content-Length"]) > ft_atoi(client_max_body) * 1024 * 1024)
// 		return (413); //413 Request Entity Too Large
// 	if(responses[fd].it2 != responses[fd].it->routes.end())
// 	{
// 		for (it4 = responses[fd].it2->methods.begin(); it4 != responses[fd].it2->methods.end(); it4++)
// 			if(*it4 == responses[fd].request.method)
// 			{
// 				if (responses[fd].request.method == "GET")
// 					return(1);
// 				if (responses[fd].request.method == "POST")
// 					return(2);
// 				if (responses[fd].request.method == "DELETE")
// 					return(3);
// 					// response_get(req, config, response);
// 				// else if (req.method == "POST")
// 				// 	response_post();
// 				// else
// 				// 	response_delete();
// 				break ;
// 			}
// 		if (it4 == responses[fd].it2->methods.end())
// 			return (405);
// 	}
// 	else
// 		return (404);
// 	return(301);
// }

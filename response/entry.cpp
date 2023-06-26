#include "../webserv.hpp"
#include "../config.hpp"

// #include <iostream> 

/**
 * true = response finished
*/


// if (request.method == "GET")
// 	status_code = 1;
// if (request.method == "POST")
// 	status_code = 2;
// if (request.method == "DELETE")
// 	status_code = 3;
// 		std::vector<Server>::iterator server_it;
// 		std::vector<Location>::iterator location_it;

// 	server_it = server(config, request);
// 	location_it = location(config, request, server_it);

// 	if (location_it == server_it->routes.end()) {
// 		status_code = 404;
// 		goto process;
// 	}

// 	if (std::find(location_it->methods.begin(), location_it->methods.end(), request.method) == location_it->methods.end()) {
// 		status_code = 405;
// 		goto process;
// 	}


// 	response.old_url = request.url;
// 	response.version = request.version;

/**
 * true = response finished
*/
int	send_response(int fd, HttpRequest& request, HttpResponse& response, int status_code, bool *close_connexion)
{
	if (!request.method.empty())
	{
		std::cout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n";
		init_response(config, response, request, fd);
		if (new_request(request, response, status_code))
			return (1);
	}
	else
	{
		continue_previous_response(response);
		std::cout << "*********************> " << response.finish_reading << std::endl;
		// response.finish_reading = 1;
		if(response.finish_reading)
		{
			if (request.headers["connection"] == "keep-alive")
				*close_connexion = false;
			return (1);
		}
	}
	return (0);
}

int new_request(HttpRequest &request, HttpResponse &response, int status_code) {

	std::string response_buffer;
	std::string content_length;

	if (!status_code)
	{
		status_code = check_req_line_headers(request);
		// std::cout << "*********************>2 " << status_code  << std::endl;
		if (status_code == 1)
		{
			if (response_get(config, response))
			{
				// std::cout << "version --" << response.version << std::endl;
				read_File(response);
				content_length = ft_tostring(response.size_file);
				// content_length = "9";
				// std::cout << "///////////" << content_length << std::endl;
				if (response.size_file == 0)
				{
					ft_send_error(404, config, response);
					return (1);
					// goto close_socket;
				} 
				else
				{
					response.headers["content-length"] = content_length;
					// response.headers["Transfer-Encoding"] = "chunked";
					// response.headers["Connection"] = "keep-alive";
					response_buffer = generate_http_response(response);
					// std::cout << "+++++++++++> " << response_buffer << std::endl;
					std::cout << "\033[33m" << "{" << response_buffer << "}" << "\033[0m"  << std::endl;
					send(response.fd, response_buffer.c_str(), response_buffer.length(), 0);
				}
			}
			else
				return (1);
				// goto close_socket;
		}
		// else if (status_code == 2)3
		// {
		// 	if(!response_post(config, response))
		// 		return (1);
		// 		// goto close_socket;
		// }
		// else if (status_code == 3)
		// {
		// 	if(!response_delete(config, response))
		// 		return (1);
		// 		// goto close_socket;
		// }
	}
	else
	{
		ft_send_error(status_code, config, response);
			return (1);
		// goto close_socket;
	}
	// else if (status_code == 2)
	// {
	// 	if(!response_post(config, response))
	// 		goto close_socket;
	// }
	// else if (status_code == 3)
	// {
	// 	if(!response_delete(config, response))
	// 		goto close_socket;
	// }
	// else
	// {
	// 	ft_send_error(status_code, config, response);
	// 	goto close_socket;
	// }
	// clients[fd] = response;
	return (0);
}

int continue_previous_response(HttpResponse &response) {

	// std::vctor
	// response.content.insert(response.content.end(), ) read_File(response);
	// response.content = "123456789";
	read_File(response);
	if (!response.finish_reading)
	{
			// std::cout << "\033[33m" << "{" << response.content.data() << "}" << "\033[0m"  << std::endl;
		// std::cout << "\033[33m" << "{" << response.content<< "}" << "\033[0m"  << std::endl;
			std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++" << std::endl;
			send(response.fd, response.content.data(), response.content.size(), 0);
			// int ret = 
			// if (ret == -EAGAIN)
				return 0;
			std::cout << "------------------SEND FAILED---------------------------------" << std::endl;
		// if (!send(response.fd, response.content.c_str(), response.content.length(), 0))
	}
	return 1;
}
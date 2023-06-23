/**
 * true = response finished
*/
int continue_previous_response(HttpResponse &response) {
	clients[fd].content = read_File(clients[fd]);
	send(fd, clients[fd].content.c_str(), clients[fd].content.length(), 0);
}

if (request.method == "GET")
	status_code = 1;
if (request.method == "POST")
	status_code = 2;
if (request.method == "DELETE")
	status_code = 3;
		std::vector<Server>::iterator server_it;
		std::vector<Location>::iterator location_it;

	server_it = server(config, request);
	location_it = location(config, request, server_it);

	if (location_it == server_it->routes.end()) {
		status_code = 404;
		goto process;
	}

	if (std::find(location_it->methods.begin(), location_it->methods.end(), request.method) == location_it->methods.end()) {
		status_code = 405;
		goto process;
	}


	response.old_url = request.url;
	response.version = request.version;

/**
 * true = response finished
*/
int new_request(HttpRequest &request, HttpResponse &response, int status_code) {


	init_response(config, response, request, fd);
	if (status_code == 1)
	{
		if (response_get(config, response))
		{
			std::cout << "version --" << response.version << std::endl;
			content_length = read_File(response);
			if (content_length == "404")
			{
				ft_send_error(404, config, response);
				goto close_socket;
			}
			else
			{
				// response.headers["content-length"] = content_length;
				response.headers["Transfer-Encoding"] = "chunked";
				response_buffer = generate_http_response(response);
				// std::cout << "+++++++++++> " << response_buffer << std::endl;
				send(response.fd, response_buffer.c_str(), response_buffer.length(), 0);
				response.content = read_File(response);
				if (response.finish_reading)
				{
					send(response.fd, response.content.c_str(), response.content.length(), 0);
					goto close_socket;
				}
			}
		}
		else
			goto close_socket;
	}
	else if (status_code == 2)
	{
		if(!response_post(config, response))
			goto close_socket;
	}
	else if (status_code == 3)
	{
		if(!response_delete(config, response))
			goto close_socket;
	}
	else
	{
		ft_send_error(status_code, config, response);
		goto close_socket;
	}
	clients[fd] = response;
}

 #include "../webserv.hpp"
#include "../config.hpp"

int	send_response(int fd, HttpRequest& request, HttpResponse& response, int status_code, bool *close_connexion)
{
	*close_connexion = false;
	if (!request.method.empty())
	{
		response.old_url = request.url;
		response.close_connexion = close_connexion;
		init_response(response, request, fd);
		if (new_request(request, response, status_code))
		{
			*close_connexion = true;
			return (1);
		}
	}
	else
	{
		int ret = read_File(response);
		// std::cout <<RED << "*******> =" << response.path_file << status_code << END << std::endl; 
		// exit(0);
		if (ret < 0) {
			*close_connexion = true;
			return 1;
		}
		if(response.finish_reading)
		{
			if (response.request.headers["Connection"] == "keep-alive")
				*response.close_connexion = false;
			else if (response.request.headers["Connection"] == "close")
				*response.close_connexion = true;
			else if (response.request.version == "HTTP/1.1")
				*response.close_connexion = true; 
			else if (response.request.version == "HTTP/2")
				*response.close_connexion = false; 
			delete_generated_file(response);
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
		if (status_code == 404)
		{
			ft_send_error(404, response);
			return (1);
		}
		if (status_code == 1)
			return (get_req(response));
		else if (status_code == 2)
			return (post_req(response));
		else if (status_code == 3)
		{
			if(!response_delete(response))
				return (1);
		}
	}
	else
	{
		ft_send_error(status_code, response);
		return (1);
	}
	return (0);
}

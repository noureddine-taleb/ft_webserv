#include "../webserv.hpp"
#include "../config.hpp"

int	response_get(HttpResponse& response)
{
	std::string type_rep;

	if (!response.location_it->creturn.to.empty())
		return (response_redirect(response));
	
	if (get_path(response))
	{
		std::cout <<YELLOW << "*******> path =" << response.path_file << END << std::endl;
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
				fill_response(302, response);
				if (response.request.url != "/")
					response.headers["Location"] = response.request.url + "/";
				else
					response.headers["Location"] = response.path_file;
				response.url_changed = true;
				std::string response_buffer = generate_http_response(response);

				if (check_connexion(response.fd) < 0)
					return (-1);
				int ret = send(response.fd, response_buffer.c_str(), response_buffer.size(), 0);
				if (ret <= 0)
					return (0);
				response.finish_reading = true;
				*response.close_connexion = true;
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

int get_req(HttpResponse &response)
{
	std::string response_buffer;
	std::string content_length;

	if (response_get(response))
	{
		read_File(response);
		content_length = ft_tostring(response.size_file);
		// if (response.size_file == 0)//////////////////////////////
		// {
		// 	ft_send_error(404, response);
		// 	return (1);
		// } 
		// else
		// {
		response.headers["content-length"] = content_length;
		response_buffer = generate_http_response(response);	
		if (check_connexion(response.fd) < 0)
			return (-1);
		int ret = send(response.fd, response_buffer.c_str(), response_buffer.length(), 0);
		if (ret < 0)
			return (1);
		// *response.close_connexion = true;
		// }
	}
	else
	{
		*response.close_connexion = true;
		return (1);
	}
	return (0);
}

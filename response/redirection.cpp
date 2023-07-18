#include "../webserv.hpp"
#include "../config.hpp"

int	response_redirect(HttpResponse& response)
{
	std::string type_rep;
	std::string	response_buffer;

	response.path_file = response.location_it->creturn.to;
	response.request.url = response.path_file ;
	response.url_changed = true;
	type_rep = type_repo(response.path_file);
	
	std::cout << YELLOW <<  type_rep << END << std::endl;
	if (type_rep == "is_file")
	{
		if (response.location_it->creturn.code)
			response_Http_Request(response.location_it->creturn.code, response);
		else
			response_Http_Request(302, response);
		return (1);
	}
	else if (type_rep == "is_directory")
	{
		 
		if (*response.request.url.rbegin() != '/')
		{
			if (response.location_it->creturn.code)
				fill_response(response.location_it->creturn.code, response);
			else
			{
				fill_response(301, response);
			}
			response.headers["Location"] = response.request.url + "/";
			std::string response_buffer = generate_http_response(response);
			if (check_connexion(response.fd) < 0)
				return (0);
			int ret = send(response.fd, response_buffer.c_str(), response_buffer.size(), 0);
			if (ret == 0)
				*response.close_connexion = true;
			if (ret < 0) {
				*response.close_connexion = false;
				return 0;
			}

			return (0);
		}
		else if (response_Http_Request(301, response))
			return (1);
	}
	else
	{
		if (response.location_it->creturn.code)
			fill_response(response.location_it->creturn.code, response);
		else
			fill_response(302, response);
		response.headers["Location"] = response.location_it->creturn.to;
		if (check_connexion(response.fd) < 0)
			return (0);
		response_buffer = generate_http_response(response);
		int ret = send(response.fd, response_buffer.c_str(), response_buffer.size(), 0);
		if (ret == 0)
			return (0);
		if (ret < 0) {
			*response.close_connexion = false;
			return 0;
		}

	};
	return (0);
}

int	response_rewrite(HttpResponse&  response)
{
	response.headers["Location"] = response.request.url;
	std::string response_buffer = generate_http_response(response);
	if (check_connexion(response.fd) < 0)
		return (0);
	int ret = send(response.fd, response_buffer.c_str(), response_buffer.size(), 0);
	if (ret == 0)
		*response.close_connexion = true;
	if (ret < 0) {
		*response.close_connexion = false;
		return 0;
	}
	return (0);

}

#include "../webserv.hpp"
#include "../config.hpp"

std::string		res_content(int status_code, HttpResponse& response)
{
	for (std::vector<ErrorPage>::iterator it = response.server_it->error_pages.begin(); it != response.server_it->error_pages.end(); it++)
	{
		if (it->error_code == status_code)
		{
			response.headers["Content-type"] = get_content_type(it->page);
			return(read_File_error(it->page, response));
		}
	}
	for (std::vector<ErrorPage>::iterator it = config.default_error_pages.begin(); it != config.default_error_pages.end(); it++)
	{		
		if (it->error_code == status_code)
		{
			response.headers["Content-type"] = get_content_type(it->page);
			return (read_File_error(it->page, response));
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
		if(response.url_changed)
			fill_response(301, response);
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


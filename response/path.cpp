#include "../webserv.hpp"
#include "../config.hpp"

void parse_path(HttpResponse &response, std::string &root)
{
	std::string target = response.location_it->target;
	std::cout << YELLOW<< "***********> response.location_it->target = " << response.location_it->target << END << std::endl;
	std::string url = response.request.url;
	size_t	find = url.find(target);

	// if (response.location_it->target == "/" && response.request.url == "/" && )
	if (*response.location_it->dir.rbegin() != '/')
		response.location_it->dir += "/" ;
	// else
	// 	response.path_file = dir;
	std::cout << PURPLE << "@@@@@@@@@@@@@@@> root ={" << root << "}" << END << std::endl;
	if (url.substr(0, find) != "" && *root.begin() != '/' )
	{
		response.path_file = url.substr(0, find)+ "/" + root + url.substr(find + target.length(), url.find("?") - 1);
		std::cout << PURPLE << "@@@@@@@@@@@@@@@> {" << response.path_file << "}" << END << std::endl;
		if (url.find("?") != std::string::npos)
			response.query_str = url.substr(url.find("?") + 1, url.length());
	}
	else
	{
		response.path_file = url.substr(0, find) + root + url.substr(find + target.length(), url.find("?") - 1);
		if (url.find("?") != std::string::npos)
			response.query_str = url.substr(url.find("?") + 1, url.length());
	}
}

int get_path(HttpResponse& response)
{
	std::string dir = response.location_it->dir;
	std::string root = response.server_it->root;
	
	if (!dir.empty())
	{
		if (response.location_it->target == "/" && response.request.url == "/")
		{
			if (*dir.rbegin() != '/')
				response.path_file = dir + "/";
			else
				response.path_file = dir;
			std::cout << YELLOW<< "***********> path file = " << response.path_file << END << std::endl;
			// exit(0);
		}
		else
			parse_path(response, dir);
		return (1);
	}
	if (!response.server_it->root.empty())
	{
		if (response.location_it->target == "/" && response.request.url == "/")
			response.path_file = root;
		else
			parse_path(response, root);
		return (1);
	}
	ft_send_error(404, response);
	return (0);
}

std::string type_repo(std::string path)
{
	struct stat info;

	if (*(path.end() - 1) == '/')
		return ("is_directory");
	if (!stat(path.c_str(), &info))
	{
		if (S_ISREG(info.st_mode))
			return ("is_file");
		if (S_ISDIR(info.st_mode))
			return ("is_directory");
	}
	return ("not found");
}


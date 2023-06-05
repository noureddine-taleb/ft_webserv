#include "webserv.hpp"
#include "config.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <dirent.h>

std::string generate_http_response(HttpResponse &res)
{
	std::stringstream res_str;
	res_str << res.version << " " << res.code<< " " << res.reason_phrase << HTTP_DEL;
	for (std::map<std::string, std::string>::iterator it = res.headers.begin(); it != res.headers.end(); it++)
		res_str << it->first << ": " << it->second << HTTP_DEL;
	res_str << HTTP_DEL;
	res_str << res.content;
	return res_str.str();
}

std::string	res_content(int status_code, HttpRequest& request, Config& config)
{
	std::vector<Server>::iterator it = server(config, request);
	if (it != config.servers.end())
	{
		for (std::vector<ErrorPage>::iterator it2 = it->error_pages.begin();it2 != it->error_pages.end(); it2++)
			if (it2->error_code == status_code)
			{
				std::cout << "hna" << std::endl;
				return(read_File(it2->page));
			}
	}
	for (std::vector<ErrorPage>::iterator it = config.default_error_pages.begin(); it != config.default_error_pages.end(); it++)
		if (it->error_code == status_code)
		{
			std::cout << "la hna" << std::endl;
			return (read_File(it->page));
		}
	return ("");
}

void response_Http_Request(int status_code, HttpRequest& request, Config& config, HttpResponse& response)
{
	switch (status_code)
	{
		case 301:
			response.reason_phrase = "Moved Permanently";
			// response.content = res_content(status_code, request, config);
			// if (response.content.empty())
			// 	response.content = read_File("www/501.html");
			// response.headers["Content-Type"] = "text/plain";
			response.headers["location"] = "https://profile.intra.42.fr/";
	}
}

// int	response_get(HttpRequest& req, Config& config)
// {
// 	DIR* directory = opendir(req.url.substr(1,req.url.length()).c_str());
// 	if (directory)
// 	{
// 		struct dirent* content_dir;
//         while ((content_dir = readdir(directory))) 
//             std::cout << "Entry name: " << content_dir->d_name << std::endl;
//         closedir(directory);
//     }
// 	else
// 		std::cout << "/////////////////////////// ma t7alch\n";
// 	return (200);
// }


// default url is "/" sot we shod met location "/"
// add return redirection
// add client_max_body_size 10M
// autoindex = on Turn on or off directory listing.
//and Set a default file to answer if the request is a directory.
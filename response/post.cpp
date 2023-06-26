// #include "../webserv.hpp"
// #include "../config.hpp"
// #include <iostream>
// #include <fstream>
// #include <sstream>
// #include <dirent.h>
// #include <cstdio>

// void	upload_exist(Config& config, HttpResponse& response, std::string& upload_path)
// {
// 	std::string type_rep;
// 	std::string response_buffer;

// 	type_rep = type_repo(upload_path);
// 	if (type_rep == "is_file" || type_rep == "not found")
// 		ft_send_error(404,config, response);
// 	std::string file_name = generate_filename();
// 	add_extention(file_name, response);
// 	std::ofstream file(file_name);
// 	if (file)
// 	{
// 		file << response.request.content;
// 		std::string destination = "uploads/" + file_name;
// 		if(!(std::rename(file_name.c_str(), destination.c_str())))
// 		{
// 			response.path_file = "www/201.html";
// 			fill_response(201, response);
// 			response.content = read_File_error(response.path_file);
// 			response.headers["content-length"] = ft_tostring(response.content.length());
// 			response_buffer = generate_http_response(response);
// 			response_buffer += response.content;
// 			send(response.fd, response_buffer.c_str(), response_buffer.length(), 0);
// 		}
// 	}
// 	else
// 		ft_send_error(404,config, response);
// }

// void	upload_not_exist(Config& config, HttpResponse& response)
// {
// 	std::string type_rep;

// 	// std::cout << "@@@@@@@@@@@@@@@@@@@@@ " << std::endl;
// 	if (get_path(config, response))
// 	{
// 		type_rep = type_repo(response.path_file);
// 		if (type_rep == "is_file")
// 		{
// 			if (response.location_it->cgi.empty())
// 			{
// 				// std::cout << "!!!!!!!!!!!!!!!!!!!! " << std::endl;
// 				ft_send_error(403, config, response);
// 			}
// 		}
// 		else if (type_rep == "is_directory")
// 		{
// 			if (response_Http_Request(301,config, response))
// 			{
// 				if (response.location_it->cgi.empty())
// 				{
// 					// std::cout << "response file = " << response.path_file << std::endl;
// 					ft_send_error(403, config, response);
// 				}
// 			}
// 		}
// 		else
// 			ft_send_error(404, config, response);
// 	}
// }

// std::string	generate_filename()
// {
// 	std::string	file_name = "file";
// 	static int num = 0;

// 	std::string num_to_str = ft_tostring(num++);
// 	file_name += "_" + num_to_str;
// 	return (file_name);
// }

// void	add_extention(std::string& filename, HttpResponse& response)
// {
// 	std::map<std::string, std::string> extention;

// 	extention[" text/html"] = ".html";
// 	extention[" text/css"] = ".css";
// 	extention[" text/javascript"] = ".js";
// 	extention[" image/png"] = ".png";
// 	extention[" application/json"] = ".json";
// 	extention[" application/xml"] = ".xml";
// 	extention[" application/pdf"] = ".pdf";
// 	extention[" image/jpeg"] = ".jpeg";
// 	extention[" image/jpeg"] = ".jpeg";
// 	extention[" image/gif"] = ".gif";
// 	extention[" text/plain"] = ".text";
// 	extention[" video/mp4"] = ".mp4";

// 	if (extention.find(response.request.headers["Content-Type"]) != extention.end())
// 		filename += extention[response.request.headers["Content-Type"]];
// }

// int response_post(Config& config, HttpResponse& response)
// {
// 	std::string upload_path = "";

// 	if (!upload_path.empty())
// 		upload_exist(config, response, upload_path);
// 	else
// 		upload_not_exist(config, response);
// 	return (0);
// }
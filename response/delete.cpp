// #include "../webserv.hpp"
// #include "../config.hpp"
// #include <iostream>
// #include <fstream>
// #include <sstream>
// #include <dirent.h>
// #include <cstdio>
// #include <cstdlib>
// #include <cstring>


// void	del_content_dir(Config config, HttpResponse& response, std::string& path_dir)
// {
// 	DIR* directory = opendir(path_dir.c_str());
// 	std::string type_rep;
// 	std::string file_path;

// 	if (directory)
// 	{
// 		struct dirent* content_dir;
// 		while ((content_dir = readdir(directory)))
// 		{
// 			if (strcmp(content_dir->d_name, ".") && strcmp(content_dir->d_name , "..")
// 				&& strcmp(content_dir->d_name, ".DS_Store"))
// 			{
// 				if (*path_dir.rbegin() != '/')
// 					path_dir += "/";
// 				file_path = path_dir + content_dir->d_name;
// 				type_rep = type_repo(file_path);
// 				if (type_rep == "is_file")
// 					unlink(file_path.c_str());
// 				else if (type_rep == "is_directory")
// 				{
// 					path_dir += "/";
// 					del_content_dir(config, response, file_path);
// 					remove(file_path.c_str());
// 				}
// 				else
// 					ft_send_error(500, config, response);
// 			}
// 		}
// 	}
// 	else
// 	{
// 		ft_send_error(403,config, response);
// 		return ;

// 	}
// 	closedir(directory);
// 	remove(path_dir.c_str());
// 	ft_send_error(204, config, response);
// }

// int response_delete(Config& config, HttpResponse& response)
// {
// 	std::string type_rep;
// 	std::string response_buffer;

// 	if (get_path(config, response))
// 	{
// 		type_rep = type_repo(response.path_file);
// 		if (type_rep == "is_file")
// 		{
// 			if (response.location_it->cgi.empty())
// 			{
// 				unlink(response.path_file.c_str());
// 				ft_send_error(204, config, response);
// 			}
// 		}
// 		if(type_rep == "is_directory")
// 		{
// 			if (*response.path_file.rbegin() != '/')
// 				ft_send_error(409, config, response);
// 			else if (response.location_it->cgi.empty())
// 				del_content_dir(config, response, response.path_file);
// 		}
// 		else
// 			ft_send_error(404, config, response);
// 	}
// 	return(0);
// }

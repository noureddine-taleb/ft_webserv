#include "../webserv.hpp"
#include "../config.hpp"

void	del_content_dir(HttpResponse& response, std::string& path_dir)
{
	DIR* directory = opendir(path_dir.c_str());
	std::string type_rep;
	std::string file_path;

	if (directory)
	{
		struct dirent* content_dir;
		while ((content_dir = readdir(directory)))
		{
			if (strcmp(content_dir->d_name, ".") && strcmp(content_dir->d_name , "..")
				&& strcmp(content_dir->d_name, ".DS_Store"))
			{
				if (*path_dir.rbegin() != '/')
					path_dir += "/";
				file_path = path_dir + content_dir->d_name;
				type_rep = type_repo(file_path);
				if (type_rep == "is_file")
					unlink(file_path.c_str());
				else if (type_rep == "is_directory")
				{
					path_dir += "/";
					del_content_dir( response, file_path);
					remove(file_path.c_str());
				}
				else
				{
					*response.close_connexion = true;
					ft_send_error(500, response);
				}
			}
		}
	}
	else
	{
		*response.close_connexion = true;
		ft_send_error(403, response);
		return ;
	}
	closedir(directory);
	remove(path_dir.c_str());
	*response.close_connexion = true;
	ft_send_error(204, response);
}

int response_delete(HttpResponse& response)
{
	std::string type_rep;
	std::string response_buffer;

	if (get_path(response))
	{
		type_rep = type_repo(response.path_file);
		if (type_rep == "is_file")
		{
				unlink(response.path_file.c_str());
				*response.close_connexion = true;
				ft_send_error(204, response);
		}
		if(type_rep == "is_directory")
		{
			if (*response.path_file.rbegin() != '/')
			{
				*response.close_connexion = true;
				ft_send_error(409, response);
			}
			else
				del_content_dir(response, response.path_file);
		}
		else
		{
			*response.close_connexion = true;
			ft_send_error(500, response);
		}
	}
	return(0);
}

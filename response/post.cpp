#include "../webserv.hpp"
#include "../config.hpp"

int fill_uplaod_file(HttpResponse &response, std::string &upload_path, std::string &file_name, std::vector<char> &content)
{
	std::string path = "upload_dir/" + file_name;
	std::ofstream file(path);
	if (file)
	{
		std::string str(content.begin(), content.end());
		file << str;
		std::string destination = upload_path + file_name;
		if (std::rename(path.c_str(), destination.c_str()))
		{
			*response.close_connexion = true;
			ft_send_error(500, response);
			file.close();
			return(0);
		}
	}
	else
	{
		*response.close_connexion = true;
		ft_send_error(404, response);
		return(0);
	}
	file.close();
	return (1);
}
void send_201_response (HttpResponse &response)
{
	std::string response_buffer;

	response.path_file = "www/201.html";
	fill_response(201, response);
	response.content_error = read_File_error(response.path_file, response);
	response.headers["content-length"] = ft_tostring(response.content_error.length());
	response_buffer = generate_http_response(response);
	response_buffer += response.content_error;
	if (check_connexion(response.fd) < 0)
	{
		*response.close_connexion = true;
		return ;
	}	
	int ret = send(response.fd, response_buffer.c_str(), response_buffer.length(), 0);
	if (ret == 0)
		*response.close_connexion = true;
	if ( ret < 0)
	{
		*response.close_connexion = false;
		response.finish_reading = false;
	}
}

void	upload_exist(HttpResponse& response, std::string& upload_path)
{
	std::string type_rep;
	std::string response_buffer;
	std::string file_name;

	type_rep = type_repo(upload_path);
	if ((response.request.files.empty() && response.request.content.empty() && response.request.vars.empty()) 
		|| type_rep == "is_file" || type_rep == "not found")
	{
		*response.close_connexion = true;
		ft_send_error(500, response);
		return ;
	}
	if (!response.request.files.empty() || !response.request.vars.empty())
	{
		if (!response.request.files.empty())
		{
			for(std::vector<File>::iterator file_it = response.request.files.begin(); file_it != response.request.files.end(); file_it++)
			{
				file_name = file_it->name;
				if(!fill_uplaod_file(response, upload_path, file_name, file_it->content))
					return ;
			}
		}
		if (!response.request.vars.empty())
		{
			for(std::vector<Var>::iterator vars_it = response.request.vars.begin(); vars_it != response.request.vars.end(); vars_it++)
			{
				file_name = vars_it->key;
				if (vars_it->key.substr(vars_it->key.find_last_of(".") + 1, vars_it->key.length()) != "txt")
					file_name += ".txt";
				if(!fill_uplaod_file(response, upload_path, file_name, vars_it->value))
					return ;
			}
		}
	}
	else if (!response.request.content.empty())
	{
		static int i;
		std::string file = "file_uplad";
		file_name = generate_filename(file, &i);
		add_extention(file_name, response);
		if(!fill_uplaod_file(response, upload_path, file_name, response.request.content))
			return ;
	}
	send_201_response(response);
}

int upload_not_exist_file(HttpResponse &response)
{
	std::string path = response.path_file;

	if (response.location_it->cgi.empty())
	{
		*response.close_connexion = true;
		ft_send_error(403, response);
		return(0);
	}
	else
	{
		check_extention(response);
		if ((response.cgi_it == response.location_it->cgi.end()
			&& (path.substr(path.find_last_of(".") + 1, path.length()) == "php"
			|| path.substr(path.find_last_of(".") + 1, path.length()) == "py")) 
			|| response.cgi_it == response.location_it->cgi.end())
			{
				*response.close_connexion = true;
				ft_send_error(403,response);
				return (0);
			}
	}
	fill_response(200, response);
	if (!response.request.content.empty())
	{
		std::ofstream content("cgi.txt");
		if (!content.is_open())
		{
			*response.close_connexion = true;
			ft_send_error(500, response);
		}
		else
		{
			response.file_name_genarated.push_back("cgi.txt");
			std::string str(response.request.content.begin(), response.request.content.end());
			content << str;
			content.close();
		}
	}
	return (execute_cgi(response));
}

int	upload_not_exist(HttpResponse& response)
{
	std::string type_rep;

	if (get_path(response))
	{
		type_rep = type_repo(response.path_file);
		if (type_rep == "is_file")
			return (upload_not_exist_file(response));
		else if (type_rep == "is_directory")
		{
			if (response_Http_Request(301, response))
				return (1);
		}
		else
		{
			*response.close_connexion = true;
			ft_send_error(404, response);
		}
	}
	return (0);
}

int post_req(HttpResponse &response)
{
	std::string response_buffer;
	std::string content_length;
	if(response_post(response))
	{
		read_File(response);
		content_length = ft_tostring(response.size_file);	
		response.headers["content-length"] = content_length;
		response_buffer = generate_http_response(response);
		if (check_connexion(response.fd) < 0)
			return (-1);
		int ret = send(response.fd, response_buffer.c_str(), response_buffer.length(), 0);
		if (ret == 0)
			*response.close_connexion = true;
		if (ret < 0)
			return (1);
	}
	else
		return (1);
	return (0);
}

int response_post(HttpResponse& response)
{
	std::string upload_path;


	if (response.location_it->upload)
	{
		if (!response.location_it->dir.empty())
		{
			if (*response.location_it->dir.rbegin() != '/')
				upload_path = response.location_it->dir + "/";
			else
				upload_path = response.location_it->dir;
		}
		else if(!response.server_it->root.empty())
		{
			if (*response.location_it->dir.rbegin() != '/')
				upload_path = response.server_it->root + "/";
			else
				upload_path = response.server_it->root;
		}
		else
		{
			*response.close_connexion = true;
			ft_send_error(404, response);
			return (0);
		}
		upload_exist(response, upload_path);
	}
	else
	{
		if (upload_not_exist(response))
			return (1);

	}
	return (0);
}
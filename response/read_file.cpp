#include "../webserv.hpp"
#include "../config.hpp"
#include <iostream>

int get_path(HttpResponse& response)
{
	std::string target = response.location_it->target;
	std::string dir = response.location_it->dir;
	std::string url = response.request.url;
	std::string root = response.server_it->root;
	
	size_t	find = url.find(target);
	if (!dir.empty())
	{
		if (url.substr(0, find) != "" && *dir.begin() != '/')
		{
			response.path_file = url.substr(0, find)+ "/" + dir + url.substr(find + target.length(), url.find("?") - 1);
			if (url.find("?") != std::string::npos)
			{
				response.query_str = url.substr(url.find("?") + 1, url.length());
				// std::cout << "\033[34m" << "query == {" << response.query_str << "}" << "\033[00m" << std::endl;
			}
			// response.query_str = url.substr(url.find("?") + 1, url.length());
			// std::cout << "\033[32m" << "query == {" << url.substr(url.find("?") + 1, url.length()) << "}" << "\033[00m" << std::endl;
		}
		else
		{
			response.path_file = url.substr(0, find) + dir + url.substr(find + target.length(), url.find("?") - 1);
			if (url.find("?") != std::string::npos)
			{
				response.query_str = url.substr(url.find("?") + 1, url.length());
				// std::cout << "\033[34m" << "query == {" << response.query_str << "}" << "\033[00m" << std::endl;
			}
		}
		return (1);
	}
	if (!response.server_it->root.empty())
	{
		if (url.substr(0, find) != "" && *dir.begin() != '/')
		{
			response.path_file = url.substr(0, find)+ "/" + root + url.substr(find + target.length(), url.find("?") - 1);
			response.query_str = url.substr(url.find("?") + 1, url.length());
		}
		else
		{
			response.path_file = url.substr(0, find) + root + url.substr(find + target.length(), url.find("?") - 1);
			response.query_str = url.substr(url.find("?") + 1, url.length());
		}
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

std::string read_File_error(std::string Path)
{
	std::ifstream file(Path);
	std::stringstream buffer;

	if (!file)
		return ("not found");
	buffer << file.rdbuf();
	return buffer.str();
}

void read_File(HttpResponse& response)
{

	std::ifstream file;
	file.open(response.path_file, std::ifstream::binary);
	std::vector<char> buffer;

	if (file.is_open())
	{
		file.seekg (0, file.end);
		int length = file.tellg();
		file.seekg (0, file.beg);

		if (response.get_length == false)
		{
			response.get_length = true;
			file.close();
			response.size_file = length;
			return ;
		}
		if (response.byte_reading < length)
		{
			int chunkSize = std::min(BUFF_SIZE, length - response.byte_reading);
			response.content.clear();
			buffer.resize(chunkSize);
			file.seekg(response.byte_reading);
			file.read(buffer.data(), chunkSize);
			ssize_t readi = file.gcount();
			response.content.assign(buffer.begin(), buffer.end());
			ssize_t i = send(response.fd,response.content.data(), readi, 0);
			// while (i < 0)
			// {
			// 	perror("client send file error");
			// 	usleep(100);
			// 	i = send(response.fd,response.content.data(), readi, 0);
			// }
			if (i > 0)
				response.byte_reading += i;
			// std::string str(response.content.begin(), response.content.end());
			// std::cout << "\033[33m" << "{" << str << "}" << "\033[0m"  << std::endl;
		}
		if (response.byte_reading == length)
		{

			response.byte_reading = 0;
			response.finish_reading = true;
			return ;
		}
		file.close();
	}
}
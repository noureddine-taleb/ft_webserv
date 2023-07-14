#include "../webserv.hpp"
#include "../config.hpp"

void parse_path(HttpResponse &response, std::string &root)
{
	std::string target = response.location_it->target;
	std::string url = response.request.url;
	size_t	find = url.find(target);

	if (url.substr(0, find) != "" && *root.begin() != '/')
	{
		response.path_file = url.substr(0, find)+ "/" + root + url.substr(find + target.length(), url.find("?") - 1);
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
		parse_path(response, dir);
		return (1);
	}
	if (!response.server_it->root.empty())
	{
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

std::string read_File_error(std::string Path)
{
	std::ifstream file(Path.c_str());
	std::stringstream buffer;

	if (!file)
		return ("not found");
	buffer << file.rdbuf();
	return buffer.str();
}

void read_File(HttpResponse& response)
{

	std::ifstream file;
	file.open(response.path_file.c_str(), std::ifstream::binary);
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
			// std::cout <<PURPLE<< "-------> start reding "<< END << std::endl;
			file.read(buffer.data(), chunkSize);
			// std::cout <<PURPLE<< "--------> end reding "<< END << std::endl;
			ssize_t readi = file.gcount();
			response.content.assign(buffer.begin(), buffer.end());
			// std::cout <<YELLOW<< "-------> start sending "<< END << std::endl;
			ssize_t i = send(response.fd,response.content.data(), readi, 0);
			// std::cout <<YELLOW<< "--------> end sending "<< END << std::endl;
			if (i < 0)
			{
				response.finish_reading = true;
				perror("send feiled");
				file.close();
				return ;	
			}
			if (i >= 0)
			{
				std::cout << YELLOW << "send work" << END << std::endl;
				response.byte_reading += i;
			}
		}
		if (response.byte_reading == length)
		{
			response.byte_reading = 0;
			response.finish_reading = true;
			file.close();
			return ;
		}
		file.close();
	}
}
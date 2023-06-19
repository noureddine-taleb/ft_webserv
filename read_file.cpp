#include "webserv.hpp"
#include "config.hpp"
#include <iostream>

int get_path(Config config, HttpResponse& response)
{
	std::string target = response.location_it->target;
	std::string dir = response.location_it->dir;
	std::string url = response.request.url;
	std::string root = response.server_it->root;
	
	size_t	find = url.find(target);
	if (!dir.empty())
	{
		response.path_file = url.substr(0, find) + dir + url.substr(find + target.length(), url.length());
		return (1);
	}
	if (!response.server_it->root.empty())
	{
		response.path_file = url.substr(0, find) + root + url.substr(find + target.length(), url.length());
		return (1);
	}
	ft_send_error(404, config, response);
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

std::string read_File(HttpResponse& response)
{

	std::ifstream file;
	std::string res = "";
	std::stringstream hex;
	file.open(response.path_file, std::ifstream::binary);
	if (file.is_open())
	{
		file.seekg (0, file.end);
		int length = file.tellg();
		file.seekg (0, file.beg);
		std::vector<char> buffer(length);

		if (response.get_length == false)
		{
			response.get_length = true;
			return (ft_tostring(length));
		}
		if (response.byte_reading < length)
		{
			// int chunkSize = std::min(BUFF_SIZE, length - byte_reading);
			int chunkSize = std::min(length, length - response.byte_reading);
			hex << std::hex << chunkSize;
			buffer.resize(chunkSize);
			res += hex.str() + "\r\n";
			file.seekg(response.byte_reading);
			file.read(buffer.data(), chunkSize);
			response.byte_reading += file.gcount();

			std::string content(buffer.begin(), buffer.end());
			res += content + "\r\n";
			if (file.gcount() == length)
			{
				hex.str("");
				chunkSize = 0;
				hex << std::hex << chunkSize;
				response.finish_reading = true;
				res += hex.str() + "\r\n" + "\r\n";
				file.close();
			}
			// if (file.gcount() == 0)
			// {
			// file.close();
			// 	return ("finish_reading");
			// }
		}
		return (res);
	}
	return ("404");
}

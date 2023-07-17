#include "../webserv.hpp"
#include "../config.hpp"

std::string read_File_error(std::string Path, HttpResponse &response)
{
	std::ifstream file(Path.c_str());
	std::stringstream buffer;

	if (!file)
	{
		fill_response(500, response);
		buffer << "<!DOCTYPE html>\n";
		buffer << "<html>\n";
		buffer << "<head>\n";
		buffer << "<title>500 Internal Server Error</title>\n";
		buffer << "</head>\n";
		buffer << "<body>\n";
		buffer << " <center>\n";
		buffer << "<h1>500 Internal Server Error</h1>\n";
		buffer << "</center>\n";
		buffer << "</body>\n";
		buffer << "</html>\n";
	}
	else
		buffer << file.rdbuf();
	return buffer.str();
}

int read_File(HttpResponse& response)
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
			return 0;
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
			if (check_connexion(response.fd) < 0)
				return (-1);
			ssize_t i = send(response.fd,response.content.data(), readi, 0);
			if (i == 0)
			{          
				file.close();
				return -1;	
			}
			if (i < 0)
			{          
				file.close();
				return 0;	
			}
			if (i > 0)
				response.byte_reading += i;
		}
		if (response.byte_reading == length)
		{
			response.byte_reading = 0;
			response.finish_reading = true;	
			file.close();
			return 0;
		}
		file.close();
	}
	else
	{
		fill_response(500, response);
		ft_send_error(500, response);
	}
	return 0;
}
 #include "../webserv.hpp"
#include "../config.hpp"

int	send_response(int fd, HttpRequest& request, HttpResponse& response, int status_code, bool *close_connexion)
{
	if (!request.method.empty())
	{
		response.old_url = request.url;
		// std::cout <<YELLOW << "*******> =" << status_code << END << std::endl; 
		response.close_connexion = close_connexion;
		init_response(response, request, fd);
		if (new_request(request, response, status_code))
		{
			return (1);
		}
	}
	else
	{
		read_File(response);
		if(response.finish_reading)
		{
			if (request.headers["connection"] == "keep-alive")
				*close_connexion = false;
			delete_generated_file(response);//////////////////////////////////
			return (1);
		}
	}
	return (0);
}

int get_req(HttpResponse &response)
{
	std::string response_buffer;
	std::string content_length;

	if (response_get(response))
	{
		read_File(response);
		content_length = ft_tostring(response.size_file);
		// if (response.size_file == 0)//////////////////////////////
		// {
		// 	ft_send_error(404, response);
		// 	return (1);
		// } 
		// else
		// {
			response.headers["content-length"] = content_length;
			response_buffer = generate_http_response(response);
			int ret = send(response.fd, response_buffer.c_str(), response_buffer.length(), 0);
			if (ret < 0)
			{
				perror("send feailed");
				// *response.close_connexion = true;
				return (1);
			}
		// }
	}
	else
		return (1);
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
		// if (response.size_file == 0)
		// {
		// 	std::cout << "...........................................\n";
		// 	ft_send_error(404, response);
		// 	return (1);
		// } 
		// else
		// {
			response.headers["content-length"] = content_length;
			response_buffer = generate_http_response(response);
			int ret = send(response.fd, response_buffer.c_str(), response_buffer.length(), 0);
			if (ret < 0)
			{
				perror("send feailed");
				// *response.close_connexion = true;
				return (1);
			}
		// }
	}
	else
		return (1);
	return (0);
}

int new_request(HttpRequest &request, HttpResponse &response, int status_code) {

	std::string response_buffer;
	std::string content_length;

	// std::cout <<YELLOW << "*******> =" << status_code << END << std::endl; 
	if (!status_code)
	{
		status_code = check_req_line_headers(request);		
		if (status_code == 1)
			return (get_req(response));
		// {
		// 	// if (response_get(response))
		// 	// {
		// 	// 	read_File(response);
		// 	// 	content_length = ft_tostring(response.size_file);
		// 	// 	if (response.size_file == 0)
		// 	// 	{
		// 	// 		ft_send_error(404, response);
		// 	// 		return (1);
		// 	// 	} 
		// 	// 	else
		// 	// 	{
		// 	// 		response.headers["content-length"] = content_length;
		// 	// 		response_buffer = generate_http_response(response);
					
		// 	// 		int ret = send(response.fd, response_buffer.c_str(), response_buffer.length(), 0);
		// 	// 		if (ret < 0)
		// 	// 		{
		// 	// 			perror("send feailed");
		// 	// 			return (1);
		// 	// 		}
		// 	// 	}
		// 	// }
		// 	else
		// 		return (1);
		// }
		else if (status_code == 2)
		{
			return (post_req(response));
			// if(response_post(response))
			// {
			// 	read_File(response);
			// 	content_length = ft_tostring(response.size_file);
			// 	if (response.size_file == 0)
			// 	{
			// 		ft_send_error(404, response);
			// 		return (1);
			// 	} 
			// 	else
			// 	{
			// 		response.headers["content-length"] = content_length;
			// 		response_buffer = generate_http_response(response);
			// 		int ret = send(response.fd, response_buffer.c_str(), response_buffer.length(), 0);
			// 		if (ret < 0)
			// 		{
			// 			perror("send feailed");
			// 			return ;
			// 		}
			// 	}
			// }
			// else
			// 	return (1);
		}
		else if (status_code == 3)
		{
			if(!response_delete(response))
				return (1);
		}
	}
	else
	{
		ft_send_error(status_code, response);
		return (1);
	}
	return (0);
}



// #include "../webserv.hpp"
// #include "../config.hpp"

// void	upload_exist(Config& config, HttpResponse& response, std::string& upload_path)
// {
// 	std::string type_rep;
// 	std::string response_buffer;

// 	type_rep = type_repo(upload_path);
// 	if (type_rep == "is_file" || type_rep == "not found")
// 		ft_send_error(501,config, response);
// 	std::string file_name = generate_filename();
// 	add_extention(file_name, response);
// 	std::ofstream file(file_name);
// 	if (file)
// 	{
// 		file << response.request.content.data();
// 		std::string destination = upload_path + file_name;
// 		if(!(std::rename(file_name.c_str(), destination.c_str())))
// 		{
// 			response.path_file = "www/201.html";
// 			fill_response(201, response);
// 			response.content_error = read_File_error(response.path_file);
// 			response.headers["content-length"] = ft_tostring(response.content_error.length());
// 			response_buffer = generate_http_response(response);
// 			response_buffer += response.content_error;
// 			send(response.fd, response_buffer.c_str(), response_buffer.length(), 0);
// 			// response.finish_reading = true;
// 		}
// 	}
// 	else
// 		ft_send_error(501,config, response);
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
// 	bool upload = true;

// 	if (upload)
// 	{
// 		std::string upload_path = response.location_it->dir + "uplaod";
// 		// std::string upload_path = "/Users/kadjane/Desktop/server" + "uplod";
// 		if (!mkdir(upload_path.c_str(), 0777))
// 		{
// 			std::cout << "++++++++++++++++++++>" << upload_path << std::endl;
// 			upload_exist(config, response, upload_path);
// 		}
// 		else
// 			std::cout << "//////////////////////////////////////////////\n";
// 	}
// 	else
// 		upload_not_exist(config, response);
// 	return (0);
// }















// int continue_previous_response(HttpResponse &response) {

// 	// std::vctor
// 	// response.content.insert(response.content.end(), ) read_File(response);
// 	// response.content = "123456789";
// 	read_File(response);
// 	// if (!response.finish_reading)
// 	// {
// 			// std::cout << "\033[33m" << "{" << response.content.data() << "}" << "\033[0m"  << std::endl;
// 		// std::cout << "\033[33m" << "{" << response.content<< "}" << "\033[0m"  << std::endl;
// 			// std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++" << std::endl;
// 			// send(response.fd, response.content.data(), response.content.size(), 0);
// 			// int ret = 
// 			// if (ret == -EAGAIN)
// 				return 0;
// 			// std::cout << "------------------SEND FAILED---------------------------------" << std::endl;
// 		// if (!send(response.fd, response.content.c_str(), response.content.length(), 0))
// 	// }
// 	// return 1;
// }






// void read_File(HttpResponse& response)
// {

// 	std::ifstream file;
// 	file.open(response.path_file, std::ifstream::binary);

// 	if (file.is_open())
// 	{
// 		file.seekg (0, file.end);
// 		int length = file.tellg();
// 		file.seekg (0, file.beg);

// 		if (response.get_length == false)
// 		{
// 			response.get_length = true;
// 			file.close();
// 			response.size_file = length;
// 			return ;
// 		}
// 		if (response.byte_reading < length)
// 		{
// 			int chunkSize = std::min(BUFF_SIZE, length - response.byte_reading);
// 			char buffer[chunkSize];
// 			// response.content.clear();
// 			// buffer.resize(chunkSize);
// 			file.seekg(response.byte_reading);
// 			file.read(buffer, chunkSize);
// 			std::string test(buffer, file.gcount());
// 			*response.out << test;
// 			response.byte_reading += file.gcount();
// 			std::cout <<"----------------------------->" << response.byte_reading << std::endl;
// 			std::cout <<"*****************************>" <<file.gcount()<< std::endl;
// 			long i = send(response.fd, test.c_str(), test.size(), 0);
// 			while (i < 0)
// 			{
// 				usleep(100);
// 				i = send(response.fd, test.c_str(), test.size(), 0);
// 			}
// 			// response.content.assign(buffer.begin(), buffer.end());
// 			// std::string str(response.content.begin(), response.content.end());
// 		}
// 		if (response.byte_reading == length)
// 		{

// 			// response.byte_reading = 0;
// 			response.finish_reading = true;
// 			return ;
// 		}
// 		file.close();
// 	}
// }
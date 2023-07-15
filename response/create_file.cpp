#include "../webserv.hpp"
#include "../config.hpp"

std::string	generate_filename(std::string &file, int *num)
{
	std::string num_to_str = ft_tostring((*num)++);
	file += "_" + num_to_str;
	return (file);
}

void	add_extention(std::string& filename, HttpResponse& response)
{
	std::map<std::string, std::string> extention;

	extention["text/html"] = ".html";
	extention["text/css"] = ".css";
	extention["text/javascript"] = ".js";
	extention["image/png"] = ".png";
	extention["application/json"] = ".json";
	extention["application/xml"] = ".xml";
	extention["application/pdf"] = ".pdf";
	extention["image/jpeg"] = ".jpeg";
	extention["image/jpeg"] = ".jpeg";
	extention["image/gif"] = ".gif";
	extention["text/plain"] = ".text";
	extention["video/mp4"] = ".mp4";

	if (extention.find(response.request.headers["Content-Type"]) != extention.end())
		filename += extention[response.request.headers["Content-Type"]];
}

void add_extention_2(std::string &file_name, std::string &content_type)
{
	std::map<std::string, std::string> extention;

	
	extention["text/html"] = ".html";
	extention["text/css"] = ".css";
	extention["text/javascript"] = ".js";
	extention["image/png"] = ".png";
	extention["application/json"] = ".json";
	extention["application/xml"] = ".xml";
	extention["application/pdf"] = ".pdf";
	extention["image/jpeg"] = ".jpeg";
	extention["image/jpeg"] = ".jpeg";
	extention["image/gif"] = ".gif";
	extention["text/plain"] = ".text";
	extention["video/mp4"] = ".mp4";

	if (extention.find(content_type) != extention.end())
		file_name += extention[content_type];
}
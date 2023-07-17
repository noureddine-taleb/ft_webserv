#include "webserv.hpp"
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <unistd.h>

extern Config config;

void parse_error_pages(std::vector<std::string> &lines,
					   std::vector<ErrorPage> &error_pages, uint32_t &i)
{
	std::string value;
	for (; i < lines.size(); i++)
	{
		std::vector<std::string> error_page = split(lines[i], ":", 1);
		assert_msg(error_page.size() == 2, "invalid line in error_pages");
		error_page[0] = trim(error_page[0]);
		error_page[1] = trim(error_page[1]);
		int error_code;
		try
		{
			error_code = ft_stoi(error_page[0]);
		}
		catch (std::invalid_argument &)
		{
			return;
		}
		ErrorPage epage;
		epage.error_code = error_code;
		epage.page = trim(error_page[1]);
		assert_msg (epage.error_code >= 400 && epage.error_code < 600, "config: error_code='" << epage.error_code << "' is not valid (valid range 400-599)\n");
		assert_msg (access(epage.page.c_str(), R_OK) == 0, "config: error_page='" << epage.page << "' is not readable\n");
		error_pages.push_back(epage);
	}
}

void parse_location(std::vector<std::string> &lines, Location &location,
					uint32_t &i)
{
	std::string value;
	std::vector<std::string> all_methods;
	all_methods.push_back("GET");
	all_methods.push_back("POST");
	all_methods.push_back("DELETE");

	value = lines[i].substr(8), value = split(value, ":")[0], value = trim(value),
	location.target = value;

	for (i++; i < lines.size(); i++)
	{
		if (lines[i].substr(0, 8) == "methods:")
		{
			value = lines[i].substr(8), value = trim(value);
			if (value == "*")
			{
				location.methods = all_methods;
			}
			else
				location.methods = split(value, " ");
		}
		else if (lines[i].substr(0, 8) == "rewrite:")
		{
			Redirection r;
			value = lines[i].substr(8), value = trim(value);
			std::vector<std::string> rewrite = split(value, " ");
			assert_msg(rewrite.size() == 2, "invalid rewrite line");
			r.from = rewrite[0];
			r.to = rewrite[1];
			location.redirections.push_back(r);
		}
		else if (lines[i].substr(0, 4) == "cgi:")
		{
			value = lines[i].substr(4), value = trim(value);
			std::vector<std::string> cgi = split(value, " ");
			assert_msg(cgi.size() == 2, "invalid cgi line");
			CGI cgi_inst;
			cgi_inst.file_extension = cgi[0];
			cgi_inst.cgi_pass = cgi[1];
			assert_msg (cgi_inst.file_extension == "php" || cgi_inst.file_extension == "py", "unsupported cgi: " << cgi_inst.file_extension << " try: py | php");
			assert_msg (access(cgi_inst.cgi_pass.c_str(), X_OK) == 0, "cgi bin is not executable");
			location.cgi.push_back(cgi_inst);
		}
		else if (lines[i].substr(0, 4) == "dir:")
		{
			value = lines[i].substr(4), value = trim(value);
			location.dir = value;
			assert_msg (access(location.dir.c_str(), F_OK) == 0, "config: dir='" << value << "' is not accessible\n");
		}
		else if (lines[i].substr(0, 6) == "index:")
		{
			value = lines[i].substr(6), value = trim(value);
			location.index = value;
		}
		else if (lines[i].substr(0, 10) == "autoindex:")
		{
			value = lines[i].substr(10), value = trim(value);
			if (value == "true")
				location.autoindex = true;
			else if (value == "false")
				location.autoindex = false;
			else
				die("unknowen value for autoindex in location scope: " + value);
		}
		else if (lines[i].substr(0, 7) == "upload:")
		{
			value = lines[i].substr(7), value = trim(value);
			if (value == "true")
				location.upload = true;
			else if (value == "false")
				location.upload = false;
			else
				die("unknowen value for upload in location scope: " + value);
		}
		else if (lines[i].substr(0, 7) == "return:")
		{
			value = lines[i].substr(7), value = trim(value);
			std::vector<std::string> parts = split(value, " ");
			assert_msg(parts.size() == 2, "invalid value for location.return\n");
			int code;
			try
			{
				code = ft_stoi(parts[0]);
			}
			catch (std::invalid_argument &)
			{
				die("invalid value for location.return code\n");
			}

			location.creturn.code = code;
			location.creturn.to = parts[1];
			assert_msg(location.creturn.code < 600, "invalid value for location.return.code\n");
		}
		else
		{
			if (location.methods.size() == 0)
				location.methods = all_methods;
			return;
		}
	}
}

void parse_server(std::vector<std::string> &lines, Server &server,
				  uint32_t &i)
{
	for (; i < lines.size();)
	{
		if (lines[i].substr(0, 13) == "server_names:")
		{
			std::string server_names = lines[i].substr(13);
			server.server_names = split(trim(server_names), " ");
			i++;
		}
		else if (lines[i].substr(0, 7) == "listen:")
		{
			std::string listen_raw = lines[i].substr(7);
			std::vector<std::string> listen = split(trim(listen_raw), ":");
			assert_msg(listen.size() == 2, "invalid listen directive");
			server.config_ip = listen[0];
			server.config_port = listen[1];
			int port;
			try
			{
				port = ft_stoi(listen[1]);
			}
			catch (std::invalid_argument &e)
			{
				die("invalid port number");
			}
			assert_msg(port >= 0 && port <= 65535, "invalid port number");
			i++;
		}
		else if (lines[i].substr(0, 8) == "location")
		{
			Location location;
			parse_location(lines, location, i);
			server.routes.push_back(location);
		}
		else if (lines[i] == "error_pages:")
		{
			parse_error_pages(lines, server.error_pages, ++i);
		}
		else if (lines[i].substr(0, 5) == "root:")
		{
			std::string value;
			value = lines[i].substr(5), value = trim(value);
			server.root = value;
			i++;
		}
		else
		{
			return;
		}
	}
}

void parse_config(std::string config_file)
{
	std::ifstream cfg(config_file.c_str());
	std::vector<std::string> lines;
	std::string line;
	std::string value;

	while (std::getline(cfg, line))
	{
		line = trim(line);
		if (line.length() == 0)
			continue;
		if (line.at(0) == '#')
			continue;
		lines.push_back(line);
	}

	for (uint32_t i = 0; i < lines.size();)
	{
		if (lines[i] == "server:")
		{
			Server server;
			parse_server(lines, server, ++i);
			config.servers.push_back(server);
		}
		else if (lines[i] == "default_error_pages:")
		{
			parse_error_pages(lines, config.default_error_pages, ++i);
		}
		else if (lines[i].substr(0, 21) == "client_max_body_size:")
		{
			value = lines[i].substr(21), value = trim(value);
			int factor = 1;
			switch (*(value.end() - 1))
			{
			case 'k':
				factor = 1024;
				break;
			case 'm':
				factor = 1024 * 1024;
				break;
			case 'g':
				factor = 1024 * 1024 * 1024;
				break;
			default:
				break;
			}
			if (factor != 1)
				value.erase(value.end() - 1);
			try
			{
				config.client_max_body_size = ft_stoi(value) * factor;
				if (config.client_max_body_size < 0)
					throw std::invalid_argument("negative number");
			}
			catch (std::invalid_argument &)
			{
				die("invalid argument for client_max_body_size");
			}
			i++;
		}
		else
			die("unknowen config in global scope ---" + lines[i]);
	}
}

void dump_config(Config config)
{
	std::cout << "client_max_body_size: " << config.client_max_body_size
			  << " byte (" << config.client_max_body_size / 1024 / 1024 << "m)"
			  << std::endl;
	for (uint32_t i = 0; i < config.servers.size(); i++)
	{
		std::cout << "server:" << std::endl;
		std::cout << "\tlisten: " << config.servers[i].config_ip << ":"
				  << config.servers[i].config_port << std::endl;

		if (config.servers[i].root.length())
			std::cout << "\troot: " << config.servers[i].root << std::endl;

		if (config.servers[i].server_names.size())
			std::cout << "\tserver_names: ";
		for (uint32_t k = 0; k < config.servers[i].server_names.size(); k++)
			std::cout << config.servers[i].server_names[k] << " ";
		if (config.servers[i].server_names.size())
			std::cout << std::endl;

		for (uint32_t j = 0; j < config.servers[i].routes.size(); j++)
		{
			std::cout << "\tlocation " << config.servers[i].routes[j].target << ":"
					  << std::endl;
			if (config.servers[i].routes[j].methods.size())
				std::cout << "\t\tmethods: ";
			for (uint32_t l = 0; l < config.servers[i].routes[j].methods.size(); l++)
				std::cout << config.servers[i].routes[j].methods[l] << " ";
			if (config.servers[i].routes[j].methods.size())
				std::cout << std::endl;

			for (uint32_t m = 0; m < config.servers[i].routes[j].redirections.size();
				 m++)
				std::cout << "\t\trewrite: "
						  << config.servers[i].routes[j].redirections[m].from << " "
						  << config.servers[i].routes[j].redirections[m].to
						  << std::endl;

			for (uint32_t n = 0; n < config.servers[i].routes[j].cgi.size(); n++)
				std::cout << "\t\tcgi: "
						  << config.servers[i].routes[j].cgi[n].file_extension << " "
						  << config.servers[i].routes[j].cgi[n].cgi_pass << std::endl;

			if (config.servers[i].routes[j].dir.length())
				std::cout << "\t\tdir: " << config.servers[i].routes[j].dir
						  << std::endl;

			if (config.servers[i].routes[j].index.length())
				std::cout << "\t\tindex: " << config.servers[i].routes[j].index
						  << std::endl;

			if (config.servers[i].routes[j].creturn.code)
				std::cout << "\t\treturn: " << config.servers[i].routes[j].creturn.code
						  << " " << config.servers[i].routes[j].creturn.to << std::endl;

			std::cout << "\t\tautoindex: "
					  << (config.servers[i].routes[j].autoindex == true ? "true"
																		: "false")
					  << std::endl;
		}

		if (config.servers[i].error_pages.size())
			std::cout << "\terror_pages:" << std::endl;
		for (uint32_t o = 0; o < config.servers[i].error_pages.size(); o++)
			std::cout << "\t\t" << config.servers[i].error_pages[o].error_code << ": "
					  << config.servers[i].error_pages[o].page << std::endl;
	}

	if (config.default_error_pages.size())
		std::cout << "default_error_pages:" << std::endl;
	for (uint32_t p = 0; p < config.default_error_pages.size(); p++)
		std::cout << "\t" << config.default_error_pages[p].error_code << ": "
				  << config.default_error_pages[p].page << std::endl;
}

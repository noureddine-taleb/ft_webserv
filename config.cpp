#include "webserv.hpp"
#include <iostream>
#include <fstream>

extern Config config;

void parse_error_pages(std::vector<std::string> &lines, std::vector<ErrorPage> &error_pages, int &i) {
	std::string value;
	for (; i < lines.size(); i++) {
		std::vector<std::string> error_page = split(lines[i], ":", 1);
		int error_code = std::stoi(error_page[0]);
		if (error_code < 0) {
			std::cout << "parsed error_code " << error_code << std::endl;
			return;
		}
		assert(error_page.size() == 2);
		
		ErrorPage epage;
		epage.error_code = error_code;
		epage.page = trim(error_page[1]);
		error_pages.push_back(epage);
	}
}

void parse_location(std::vector<std::string> &lines, Location &location, int &i) {
	std::string value;
	for (; i < lines.size(); i++) {
		if (lines[i].substr(0, 8) == "methods:") {
			value = lines[i].substr(8), value = trim(value);
			location.methods = split(value, " ");
		} else if (lines[i].substr(0, 8) == "rewrite:") {
			Redirection r;
			value = lines[i].substr(8), value = trim(value);
			std::vector<std::string> rewrite = split(value, " ");
			assert(rewrite.size() == 2);
			r.from = rewrite[0];
			r.to = rewrite[1];
			location.redirections.push_back(r);
		} else if (lines[i].substr(0, 4) == "cgi:") {
			value = lines[i].substr(4), value = trim(value);
			std::vector<std::string> cgi = split(value, " ");
			assert(cgi.size() == 2);
			location.cgi.file_extension = cgi[0];
			location.cgi.cgi_pass = cgi[1];
		} else if (lines[i].substr(0, 5) == "root:") {
			value = lines[i].substr(5), value = trim(value);
			location.root = value;
		} else if (lines[i].substr(0, 6) == "index:") {
			value = lines[i].substr(6), value = trim(value);
			location.index = value;
		} else if (lines[i].substr(0, 8) == "listing:") {
			value = lines[i].substr(8), value = trim(value);
			if (value == "true")
				location.list_dir_content = true;
			else if (value == "false")
				location.list_dir_content = false;
			else
				die("unknowen value for listing in location scope: " + value);
		} else {
			std::cout << "parse_location done on " << lines[i] << std::endl;
			return;
		}
	}
}

void parse_server(std::vector<std::string> &lines, Server &server, int &i) {
	for (; i < lines.size(); i++) {
start:
		if (lines[i].substr(0, 13) == "server_names:") {
			std::string server_names = lines[i].substr(13);
			server.server_names = split(trim(server_names), " ");
		} else if (lines[i].substr(0, 7) == "listen:") {
			std::string listen_raw = lines[i].substr(7);
			std::vector<std::string> listen = split(trim(listen_raw), ":");
			assert(listen.size() == 2);
			server.ip = listen[0];
			server.port = std::stoi(listen[1]);
			assert(server.port > 0);
		} else if (lines[i] == "location:") {
			Location location;
			parse_location(lines, location, ++i);
			server.routes.push_back(location);
			goto start;
		} else if (lines[i] == "error_pages:") {
			parse_error_pages(lines, server.error_pages, ++i);
			goto start;
		} else {
			std::cout << "parse_server done on " << lines[i] << std::endl;
			return;
		}
	}
}

/**
 * TODO:
 * ignore empty lines
 * check if a block is empty
 * check required blocks
*/
void parse_config(std::string config_file) {
	std::ifstream cfg(config_file);
	std::vector<std::string> lines;
	std::string line;
	while (std::getline(cfg, line)) {
		line = trim(line);
		if (line.length() == 0)
			continue;
		if (line.at(0) == '#')
			continue;
		lines.push_back(line);
	}

	for (int i = 0; i < lines.size(); i++) {
start_config:
		if (i >= lines.size())
			break;
		if (lines[i] == "server:") {
			Server server;
			parse_server(lines, server, ++i);
			config.servers.push_back(server);
			goto start_config;
		} else if (lines[i] == "default_error_pages:") {
			parse_error_pages(lines, config.default_error_pages, ++i);
			goto start_config;
		} else
			die("unknowen config in global scope ---" + lines[i] + "--- " + lines[i-1]);
	}
}

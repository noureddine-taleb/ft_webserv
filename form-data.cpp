#include "webserv.hpp"

/**
 * @return
 * -1: error
 * 0: success
*/
int parse_form_data_headers(std::vector<char> &body, std::map<std::string, std::string> &headers, std::string delimiter, bool &done) {
	std::string delstr("--" + delimiter);
	std::vector<char>::iterator del_position = find(delstr, body);
	if (del_position != body.begin()) {
		debug("files: headers: no delimiter at the begining");
		return -1;
	}
	body.erase(body.begin(), body.begin() + delstr.length());
	// the end of body
	if (find("--", body) == body.begin()) {
		if (body.size() != 4)
			debug("files: warn: more content after the end landmark");
		debug("files: done parsing headers");
		done = true;
		return 0;
	}
	body.erase(body.begin(), body.begin() + HTTP_DEL_LEN);
	while (body.size()) {
		std::vector<char>::iterator header_end = find(HTTP_DEL, body);
		if (header_end == body.end()) {
			debug("files: headers: missing delimiter");
			return -1;
		}
		std::string header(body.begin(), header_end);
		debug("files: parsing header " + header);
		body.erase(body.begin(), body.begin() + header.length() + HTTP_DEL_LEN);
		if (header.length() == 0)
			break;
		std::vector<std::string> headerv = split(header, ":", 1);
		if (headerv.size() != 2) {
			debug("files: headers: malformed header line");
			return -1;
		}
		headerv[0] = trim(headerv[0]);
		headerv[1] = trim(headerv[1]);
		headers[headerv[0]] = headerv[1];
	}
	return 0;
}

/**
 * @return
 * -1: error
 * 0: success
*/
int parse_form_data_content(std::vector<char> &body, std::vector<char> &content, std::string delimiter) {
	std::string delstr(HTTP_DEL + std::string("--") + delimiter);
	std::vector<char>::iterator del_position = find(delstr, body);
	if (del_position == body.end()) {
		return -1;
	}
	content = std::vector<char>(body.begin(), del_position);
	body.erase(body.begin(), del_position + HTTP_DEL_LEN);
	return 0;
}
/**
 * Content-Type: multipart/form-data; key=val; key2=val2
 * @return
 * -1: error
 * 0: success
*/
int parse_form_data_header_vars(std::string value, std::map<std::string, std::string> &vars) {
	std::vector<std::string> parts = split(value, ";");
	parts.erase(parts.begin(), parts.begin() + 1);
	for (std::vector<std::string>::iterator i = parts.begin(); i != parts.end(); i++) {
		std::vector<std::string> key_value = split(*i, "=", 1);
		if (key_value.size() != 2)
			return -1;
		key_value[0] = trim(key_value[0]);
		key_value[1] = trim(key_value[1]);
		if (!key_value[0].length() || key_value[1].length() < 2)
			return -1;
		key_value[1] = std::string(key_value[1].begin() + 1, key_value[1].end() - 1); // remove quotes
		vars[key_value[0]] = key_value[1];
	}
	return 0;
}

int parse_form_data_files(HttpRequest &request) {
	debug("files: parse_form_data_files called");

	if (!request.headers.count("Content-Type")) {
		debug("files: missing Content-Type");
		return 0;
	}
	std::vector<std::string> parts = split(request.headers["Content-Type"], ";");
	if (trim(parts[0]) != "multipart/form-data") {
		debug("files: Content-Type != multipart/form-data");
		return 0;
	}
	std::string boundary = trim(parts[1]);
	std::vector<std::string>key_value = split(boundary, "=", 1);
	if (key_value[0] != "boundary") {
		debug("multipart/form-data: another variable '" + key_value[0] + "' other than 'boundary' is provided");
		return 0;
	}
	std::string delimiter = key_value[1];
	std::vector<char> body = request.content;
	std::vector<File> files;
	std::map<std::string, std::string> headers;

	debug("files: start looking for files");

	while (body.size()) {
		File f;
		int skip = 0;
		bool done = false;
		std::map<std::string, std::string> vars;
		int ret = parse_form_data_headers(body, headers, delimiter, done);
		if (done)
			break;
		if (ret < 0) {
			debug("files: bad headers");
			return -1;
		}
		if (!headers.count("Content-Disposition") || !headers.count("Content-Type") || headers["Content-Type"] != "application/octet-stream") {
			debug("files: unavailable header skip");
			skip = 1;
		}
		if (parse_form_data_header_vars(headers["Content-Disposition"], vars) < 0) {
			debug("files: parse_form_data_header_vars");
			return -1;
		}
		if (!vars.count("filename")) {
			debug("files: missing filename var skip");
			skip = 1;
		}
		if (parse_form_data_content(body, f.content, delimiter) < 0) {
			debug("files: error parse_form_data_content");
			return -1;
		}
		if (!skip) {
			debug("files: a file parsed successfully");
			f.name = vars["filename"];
			files.push_back(f);
		}
	}
	std::cout << "files: found " << files.size() << std::endl;
	request.files = files;
	return 0;
}

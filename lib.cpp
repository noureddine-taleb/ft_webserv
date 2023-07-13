#include <iostream>
#include <string>
#include <map>
#include <sstream>
#include <vector>
#include <cstring>
#include <limits.h>
#include "webserv.hpp"

std::vector<std::string> split(std::string s, std::string delimiter, unsigned int max_splits) {
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    std::string token;
    std::vector<std::string> res;

    while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos && max_splits-- > 0) {
        token = s.substr(pos_start, pos_end - pos_start);
        res.push_back(token);
        pos_start = pos_end + delim_len;
    }

    res.push_back(s.substr(pos_start));
    return res;
}

std::string trim(std::string s)
{
    std::string::iterator start = s.begin();
    // todo remove isspace
    while (start != s.end() && std::isspace(*start)) {
        start++;
    }
 
    std::string::iterator end = s.end();
    do {
        end--;
    } while (std::distance(start, end) > 0 && std::isspace(*end));
 
    return std::string(start, end + 1);
}

std::vector<char>::iterator find(std::string str, std::vector<char> &vec) {
	void *pos = memmem(&vec[0], vec.size(), str.data(), str.length());

	if (pos != NULL)
		return vec.begin() + ((char *)pos - &vec[0]);
	return vec.end();
}

int stoi(std::string str) {
    if (str.length() == 0)
        throw std::invalid_argument("number empty");
    for (unsigned int i = 0; i < str.length(); i++) {
        if (!isdigit(str[i]))
            throw std::invalid_argument("not number");
    }

    long res = std::atol(str.c_str());
    std::stringstream ss;
    ss << res;
    if (ss.str() != str)
        throw std::invalid_argument("overflow");
    if (res > INT_MAX || res < INT_MIN)
        throw std::invalid_argument("overflow");
    return (int)res;
}
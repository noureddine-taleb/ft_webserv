#include <iostream>
#include <string>
#include <map>
#include <sstream>
#include <vector>
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
    while (start != s.end() && std::isspace(*start)) {
        start++;
    }
 
    std::string::iterator end = s.end();
    do {
        end--;
    } while (std::distance(start, end) > 0 && std::isspace(*end));
 
    return std::string(start, end + 1);
}

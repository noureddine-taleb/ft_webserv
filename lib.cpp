#include <iostream>
#include <string>
#include <map>
#include <sstream>
#include <vector>
#include <cstring>
#include <limits.h>
#include "webserv.hpp"

std::vector<std::string> split(std::string s, std::string delimiter, unsigned int max_splits)
{
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    std::string token;
    std::vector<std::string> res;

    while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos && max_splits-- > 0)
    {
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
    while (start != s.end() && std::isspace(*start))
    {
        start++;
    }

    std::string::iterator end = s.end();
    do
    {
        end--;
    } while (std::distance(start, end) > 0 && std::isspace(*end));

    return std::string(start, end + 1);
}

void * ft_memmem(const void *l, size_t l_len, const void *s, size_t s_len)
{
	char *cur, *last;
	const char *cl = (const char *)l;
	const char *cs = (const char *)s;

	/* we need something to compare */
	if (l_len == 0 || s_len == 0)
		return NULL;

	/* "s" must be smaller or equal to "l" */
	if (l_len < s_len)
		return NULL;

	/* special case where s_len == 1 */
	// if (s_len == 1)
	// 	return std::memchr(l, (int)*cs, l_len);

	/* the last position where its possible to find "s" in "l" */
	last = (char *)cl + l_len - s_len;

	for (cur = (char *)cl; cur <= last; cur++)
		if (cur[0] == cs[0] && std::memcmp(cur, cs, s_len) == 0)
			return cur;

	return NULL;
}

std::vector<char>::iterator find(std::string str, std::vector<char> &vec)
{
    void *pos = ft_memmem(&vec[0], vec.size(), str.data(), str.length());

    if (pos != NULL)
        return vec.begin() + ((char *)pos - &vec[0]);
    return vec.end();
}

int ft_stoi(std::string str)
{
	if (str.length() == 0)
		throw std::invalid_argument("number empty");
	for (unsigned int i = 0; i < str.length(); i++)
	{
		if (!std::isdigit(str[i]))
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

unsigned int ft_stoi_base_16(std::string str) {
	if (str.length() == 0)
		throw std::invalid_argument("number empty");
	else if (str.length() > 8)
		throw std::invalid_argument("number overflow");
	for (unsigned int i = 0; i < str.length(); i++)
	{
		if (!std::isdigit(str[i]) && !(str[i] >= 'a' || str[i] <= 'f'))
			throw std::invalid_argument("not number");
	}
	unsigned int x;
	std::stringstream ss;
	ss << std::hex << str;
	ss >> x;
	return x;
}
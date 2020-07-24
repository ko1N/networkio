
#ifndef __STRINGUTIL_H__
#define __STRINGUTIL_H__

#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace std {
std::vector<std::string> tokenize(const std::string &in, std::string sep);
std::vector<std::string> tokenize(std::string s, char c);
std::string strip(std::string s, char c);
std::string strip_all(const std::string &s, const std::string &c);
std::map<std::string, std::string> tojson(std::string s);
std::string replace_all(std::string &str, const std::string &from, const std::string &to);
bool filter_ip(char c);
} // namespace std

#endif

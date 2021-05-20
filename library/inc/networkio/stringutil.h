
#ifndef __STRINGUTIL_H__
#define __STRINGUTIL_H__

#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace std {
auto tokenize(const std::string &str, char c) -> std::vector<std::string>;
auto tokenize(const std::string &str, const std::string &delimiter) -> std::vector<std::string>;
auto strip(const std::string &s, char c) -> std::string;
auto strip_all(const std::string &s, const std::string &c) -> std::string;
auto tojson(const std::string &s) -> std::map<std::string, std::string>;
auto replace_all(std::string &str, const std::string &from, const std::string &to) -> std::string;
auto filter_ip(char c) -> bool;
} // namespace std

#endif

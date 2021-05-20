#include <string.h>

#include <networkio/stringutil.h>

std::vector<std::string> std::tokenize(const std::string &str, char c) {
  std::vector<std::string> vTokens;
  std::string strItem;
  std::stringstream strstr(str);
  while (std::getline(strstr, strItem, c)) {
    if (strItem != "")
      vTokens.push_back(strItem);
  }

  return vTokens;
}

std::vector<std::string> std::tokenize(const std::string &str,
                                       const std::string &delimiter) {
  size_t pos_start = 0, pos_end, delim_len = delimiter.length();
  string token;
  std::vector<std::string> res;

  while ((pos_end = str.find(delimiter, pos_start)) != std::string::npos) {
    token = str.substr(pos_start, pos_end - pos_start);
    pos_start = pos_end + delim_len;
    res.push_back(token);
  }

  res.push_back(str.substr(pos_start));
  return res;
}

std::string std::strip(const std::string &s, char c) {
  std::string s2 = s;
  while (s2.find(c) != std::string::npos)
    s2.erase(s2.begin() + s2.find(c));
  return s2;
}

/*
TODO: implement faster method
*/
std::string std::strip_all(const std::string &s, const std::string &c) {
  std::string r = s;
  for (auto &_c : c) {
    r = std::strip(r, _c);
  }
  return r;
}

std::map<std::string, std::string> std::tojson(const std::string &s) {
  std::map<std::string, std::string> r;
  std::string s2 = strip(
      strip(strip(strip(strip(strip(strip(s, '\n'), '\r'), '"'), ' '), 9), '{'),
      '}');

  std::vector<std::string> v = tokenize(s2, ',');
  for (size_t i = 0; i < v.size(); i++) {
    std::vector<std::string> v2 = tokenize(v[i], ':');
    if (v2.size() >= 2)
      r.insert(std::pair<std::string, std::string>(v2[0], v2[1]));
  }

  return r;
}

std::string std::replace_all(std::string &str, const std::string &from,
                             const std::string &to) {
  if (from.empty())
    return "";

  std::string strcopy = str;

  size_t start_pos = 0;
  while ((start_pos = strcopy.find(from, start_pos)) != std::string::npos) {
    strcopy.replace(start_pos, from.length(), to);
    start_pos += to.length(); // In case 'to' contains 'from', like replacing
                              // 'x' with 'yx'
  }

  return strcopy;
}

bool std::filter_ip(char c) {
  return !(c >= 0 /*&& c < 128*/ && (isdigit(static_cast<int>(c)) || c == '.'));
}

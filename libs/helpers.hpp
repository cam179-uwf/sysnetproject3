#ifndef H_HELPERS
#define H_HELPERS

#include <string>
#include <map>
#include <vector>
#include "../libs/user-info.hpp"

std::string base64_encode(const std::string& str);
std::string generate_token(const std::string& username);
std::string read_file_contents(const std::string& path);
bool is_whitespace(const std::string& str);
void save_file(std::vector<UserInfo>& users);
void load_file(std::vector<UserInfo>& users);

#endif

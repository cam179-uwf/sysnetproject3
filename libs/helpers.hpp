/**
 * System & Networks II (COP4635)
 * Christian Marcellino, Ryan Waddington
 * 4/15/2025
 * 
 * For helper methods used for the server/client.
 */

#ifndef H_HELPERS
#define H_HELPERS

#include <string>
#include <map>
#include <vector>
#include "../libs/user-info.hpp"

/// @brief Encode a string as base64.
/// @param str The string to encode.
/// @return A string representing the original but in base64.
std::string base64_encode(const std::string& str);

/// @brief Generates a random token using a username.
/// @return A randomly generated token. 
std::string generate_token(const std::string& username);

/// @brief Reads the entire file contents of a specified path.
/// @return The entire file contents. 
std::string read_file_contents(const std::string& path);

/// @brief Determine if a string is only made up of whitespace.
/// @return True if the string is only made up of whitespace, false otherwise. 
bool is_whitespace(const std::string& str);

/// @brief Save user data to a file.
void save_file(std::vector<UserInfo>& users);

/// @brief Load user data from a file. 
void load_file(std::vector<UserInfo>& users);

#endif

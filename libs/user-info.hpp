/**
 * Christian Marcellino, Ryan Waddington
 * 4/15/2025
 * 
 * Used for maintaining user info.
 */

#ifndef H_USER_INFO
#define H_USER_INFO

#include <string>
#include <vector>

struct UserInfo
{
    std::string username;
    std::string password;
    std::vector<std::string> locations;
    int fd;

    /// @brief Creates a string of all the locations a user has.
    std::string locations_to_string();
};

#endif
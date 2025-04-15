/**
 * Christian Marcellino
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

    std::string locations_to_string();
};

#endif
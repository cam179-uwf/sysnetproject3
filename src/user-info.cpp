/**
 * Christian Marcellino
 * 4/15/2025
 * 
 * Used for maintaining user info.
 */

#include "../libs/user-info.hpp"

std::string UserInfo::locations_to_string()
{
    std::string result;

    for (auto str : locations)
    {
        result += str + "\n";
    }

    return result;
}
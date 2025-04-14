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
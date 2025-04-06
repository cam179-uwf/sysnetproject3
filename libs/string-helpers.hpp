#ifndef H_STRING_HELPERS
#define H_STRING_HELPERS

#include <vector>
#include <string>
#include <cwctype>

namespace strhelp
{
    std::vector<std::string> split(const std::string& text, const char delimiter);
    std::string trim(const std::string& text);
}

#endif
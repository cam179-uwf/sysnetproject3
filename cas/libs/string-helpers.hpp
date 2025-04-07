/**
 * Christian Marcellino
 * 4/7/2025
 * 
 * This file is for any string extensions.
 */

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
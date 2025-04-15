/**
 * Christian Marcellino, Ryan Waddington
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
    /// @brief Splits a string by a delimiter.
    /// @param text The string to split.
    /// @param delimiter The delimiter.
    /// @return An array of strings representing the split pieces.
    std::vector<std::string> split(const std::string& text, const char delimiter);

    /// @brief Splits a string by a delimiter.
    /// @param text The string to split.
    /// @param delimiter The delimiter.
    /// @param maxNumberOfSplits Limits the number of splits to a max.
    /// @return An array of strings representing the split pieces.
    std::vector<std::string> split(const std::string& text, const char delimiter, size_t maxNumberOfSplits);

    /// @brief Removes white spaces from the beginning and end of the string.
    /// @param text The string to remove whitespaces from.
    /// @return A string with the whitespaces removed from the beginning and end of the string.
    std::string trim(const std::string& text);
}

#endif
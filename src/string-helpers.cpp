#include "../libs/string-helpers.hpp"

namespace strhelp 
{
    std::vector<std::string> split(const std::string& text, const char delimiter)
    {
        std::vector<std::string> result;
        size_t start = 0, end = 0;

        for (size_t i = 0; i < text.size(); ++i)
        {
            end = i;

            if (text[i] == delimiter)
            {
                result.push_back(text.substr(start, end - start));

                start = i + 1;
            }
        }

        if (start != (end + 1) && start < text.size())
        {
            result.push_back(text.substr(start, (end + 1) - start));
        }

        return result;
    }

    std::string trim(const std::string& text)
    {
        size_t start = 0, end = text.size() - 1;

        bool startIsWS = iswspace(text[start]);
        bool endIsWS = iswspace(text[end]);

        while (startIsWS || endIsWS)
        {
            if (startIsWS)
            {
                ++start;
            }
            else if (endIsWS)
            {
                --end;
            }

            startIsWS = iswspace(text[start]);
            endIsWS = iswspace(text[end]);
        }

        return text.substr(start, (end + 1) - start);
    }
}

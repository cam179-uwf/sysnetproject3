/**
 * System & Networks II (COP4635)
 * Christian Marcellino, Ryan Waddington
 * 4/15/2025
 * 
 * For handling HTTP requests.
 */

#include "../libs/http-request.hpp"

#include <sstream>

#include "../libs/string-helpers.hpp"

void cas::HttpRequest::init_from_raw_http_header(const std::string& rawHttpHeader)
{
    std::istringstream iss(rawHttpHeader);

    std::string firstLine;
    std::getline(iss, firstLine);

    auto firstLineSplits = strhelp::split(firstLine, ' ', 3);

    if (firstLineSplits.size() >= 1)
    {
        method = firstLineSplits[0];
    }

    if (firstLineSplits.size() >= 2)
    {
        path = firstLineSplits[1];
    }

    if (firstLineSplits.size() >= 3)
    {
        protocol = firstLineSplits[2];
    }

    std::string header;
    std::getline(iss, header);

    while (header.size() > 0 && !iswspace(header[0]))
    {
        auto headerSplits = strhelp::split(header, ':');

        if (headerSplits.size() == 2)
        {
            headers[headerSplits[0]] = strhelp::trim(headerSplits[1]);
        }

        std::getline(iss, header);
    }
}

bool cas::HttpRequest::headers_contain(const std::string& key) const
{
    return headers.find(key) != headers.end();
}

bool cas::HttpRequest::try_get_header(const std::string& key, std::string& outValue)
{
    if (headers_contain(key))
    {
        outValue = headers[key];
        return true;
    }

    return false;
}

std::string cas::HttpRequest::to_string() const
{
    std::ostringstream oss;

    oss << method << " " << path << " " << protocol << "\r\n";

    for (auto header : headers)
    {
        oss << header.first << ": " << header.second << "\r\n";
    }

    oss << "\r\n";
    oss << body;

    return oss.str();
}

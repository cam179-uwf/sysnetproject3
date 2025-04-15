/**
 * Christian Marcellino, Ryan Waddington
 * 4/7/2025
 * 
 * For handling HTTP requests.
 */

#ifndef H_HTTP_REQUEST
#define H_HTTP_REQUEST

#include <string>
#include <map>

namespace cas 
{
    struct HttpRequest
    {
        std::string method = "GET";
        std::string path = "/";
        std::string protocol = "HTTP/1.1";
        std::map<std::string, std::string> headers;
        std::string body = "";

        void init_from_raw_http_header(const std::string& rawHttpHeader);
        bool headers_contain(const std::string& key) const;
        bool try_get_header(const std::string& key, std::string& outValue);
        std::string to_string() const;
    };
}

#endif
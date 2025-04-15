/**
 * System & Networks II (COP4635)
 * Christian Marcellino, Ryan Waddington
 * 4/15/2025
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

        /// @brief Initiates request fields using a raw http header.
        void init_from_raw_http_header(const std::string& rawHttpHeader);

        /// @brief Determines if the headers contain the specified key.
        /// @return True if the key exists in the headers, false otherwise.
        bool headers_contain(const std::string& key) const;

        /// @brief Tries to get the header with the specified key.
        /// @param outValue The referenced value to set if the specified key is found. 
        /// @return True if the key is found, otherwise false.
        bool try_get_header(const std::string& key, std::string& outValue);

        /// @return A string representing the raw http request.
        std::string to_string() const;
    };
}

#endif
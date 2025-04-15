/**
 * Christian Marcellino, Ryan Waddington
 * 4/7/2025
 * 
 * This file is for our client http logic.
 */

#ifndef H_HTTP_CLIENT
#define H_HTTP_CLIENT

#include <string>
#include <future>
#include <map>
#include "fd-reader.hpp"

#define DEFAULT_CLIENT_BUFFER_SIZE 4096

namespace cas
{
    /// @brief Handles server responses.
    struct HttpClientResponse
    {
        std::string httpVersion;
        int statusCode;
        std::string statusMessage;
        std::map<std::string, std::string> headers;
        std::string body;

        /// @brief Gets the raw response as a string.
        /// @return The raw response.
        std::string to_string();
    };

    /// @brief Handles client requests.
    struct HttpClientRequest
    {
        std::string path = "/";
        std::map<std::string, std::string> headers;
        std::string body;

        /// @brief Gets the raw request as a string.
        /// @return The raw request.
        std::string to_string();
    };

    /// @brief Connects to an address and handles client responses and requests.
    class HttpClient
    {
        std::string _host;
        int _port;
        int _bufferSize;
        int _clientFd;

        /// @brief Makes an HTTP request using the provided method and request.
        /// @param method The HTTP method to use.
        /// @param request The HTTP request.
        /// @return A HttpClientResponse.
        /// @throw ClientException
        HttpClientResponse make_request(const std::string& method, const HttpClientRequest& request);

    public:
        HttpClient(const std::string& host, const int port, const int bufferSize);
        ~HttpClient();
        HttpClient(const HttpClient& other) = default;
        HttpClient& operator=(const HttpClient& other) = default;

        /// @brief Send a get request message to the server.
        /// @param request The request.
        /// @return The response from the server.
        /// @throw ClientException
        std::future<HttpClientResponse> get_async(const HttpClientRequest& request);

        /// @brief Send a post request message to the server.
        /// @param request The request.
        /// @return The response from the server.
        /// @throw ClientException
        std::future<HttpClientResponse> post_async(const HttpClientRequest& request);
    };
}

#endif
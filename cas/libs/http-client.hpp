/**
 * System & Networks II (COP4635)
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
#include "../libs/http-request.hpp"
#include "../libs/http-response.hpp"

#define DEFAULT_CLIENT_BUFFER_SIZE 4096

namespace cas
{
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
        HttpResponse make_request(const std::string& method, const HttpRequest& equest);

    public:
        HttpClient(const std::string& host, const int port, const int bufferSize);
        ~HttpClient();
        HttpClient(const HttpClient& other) = default;
        HttpClient& operator=(const HttpClient& other) = default;

        /// @brief Send a get request message to the server.
        /// @param request The request.
        /// @return The response from the server.
        /// @throw ClientException
        std::future<HttpResponse> get_async(const HttpRequest& request);

        /// @brief Send a post request message to the server.
        /// @param request The request.
        /// @return The response from the server.
        /// @throw ClientException
        std::future<HttpResponse> post_async(const HttpRequest& request);
    };
}

#endif
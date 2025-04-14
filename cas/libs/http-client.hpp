/**
 * Christian Marcellino
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

// cas (Client and Server)
namespace cas
{
    struct HttpClientResponse
    {
        std::string httpVersion;
        int statusCode;
        std::string statusMessage;
        std::map<std::string, std::string> headers;
        std::string body;

        std::string to_string();
    };

    struct HttpClientRequest
    {
        std::string path = "/";
        std::map<std::string, std::string> headers;
        std::string body;

        std::string to_string();
    };

    class HttpClient
    {
        std::string _host;
        int _port;
        int _bufferSize;
        int _clientFd;

        HttpClientResponse make_request(const std::string& method, const HttpClientRequest& request);

    public:
        HttpClient(const std::string& host, const int port, const int bufferSize);
        ~HttpClient();
        HttpClient(const HttpClient& other) = default;
        HttpClient& operator=(const HttpClient& other) = default;

        std::future<HttpClientResponse> get_async(const HttpClientRequest& request);
        std::future<HttpClientResponse> post_async(const HttpClientRequest& request);
    };
}

#endif
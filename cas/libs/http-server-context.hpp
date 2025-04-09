/**
 * Christian Marcellino
 * 4/7/2025
 * 
 * This file is for our server http context logic.
 */

#ifndef H_HTTP_SERVER_CONTEXT
#define H_HTTP_SERVER_CONTEXT

#include <string>
#include <map>
#include <future>

namespace cas
{
    /// @brief For keeping track of requests
    struct HttpRequest
    {
        std::string get_method() const;
        std::string get_path() const;
        std::string get_protocol() const;
        std::map<std::string, std::string> get_headers() const;
        std::string get_body() const;

        void parse(const std::string& content);

        std::string to_string();

    private:
        std::string _method;
        std::string _path;
        std::string _protocol;
        std::map<std::string, std::string> _headers;
        std::string _body;
    };

    /// @brief For keeping track of and sending responses
    struct HttpResponse
    {
        std::string protocol = "HTTP/1.1";
        size_t statusCode = 200;
        std::string statusMessage = "Success";
        std::map<std::string, std::string> headers;
        std::string body;

        std::future<void> sendoff_async();
        // std::future<void> send_dont_close_async();

        std::string to_string();

        HttpResponse(int clientFd);

    private:
        int _clientFd;
    };

    /// @brief For holding the context of a server HTTP transaction
    struct HttpServerContext
    {
        HttpRequest request;
        HttpResponse response = HttpResponse(0);
        
        void set_client_fd(const int clientFd);
        int get_client_fd() const;

    private:
        int _clientFd = 0;
    };
}

#endif
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
        std::string protocol;
        size_t statusCode;
        std::string statusMessage;
        std::map<std::string, std::string> headers;
        std::string body;

        void set_client_fd(int client_fd);
        std::future<void> send_and_close_async();

    private:
        int _clientFd;
    };

    /// @brief For holding the context of a server HTTP transaction
    struct HttpServerContext
    {
        HttpRequest request;
        HttpResponse response;
    };
}

#endif
#ifndef H_HTTP_SERVER
#define H_HTTP_SERVER

#include <future>
#include <vector>
#include <string>
#include <error.h>
#include <map>

// cas (Client and Server)
namespace cas 
{
    class ServerException : public std::runtime_error
    {
    public:
        using std::runtime_error::runtime_error;
    };

    struct HttpRequest
    {
        std::string get_method() const;
        std::string get_path() const;
        std::string get_protocol() const;
        std::map<std::string, std::string> get_headers() const;
        std::string get_body() const;

        void parse(const char* buffer, size_t length);

    private:
        std::string _method;
        std::string _path;
        std::string _protocol;
        std::map<std::string, std::string> _headers;
        std::string _body;
    };

    struct HttpResponse 
    {
        void set_client_fd(int client_fd);
        void send_and_close();

    private:
        int _clientFd;
    };

    class HttpServerContext
    {
        HttpRequest _request;
        HttpResponse _response;

    public:
        HttpServerContext();
        ~HttpServerContext();
        HttpServerContext(const HttpServerContext& other);
        HttpServerContext& operator=(const HttpServerContext& other);

        HttpRequest& get_req();
        HttpResponse& get_res();
    };

    /// @brief Basically an HTTP Listener
    class HttpServer 
    {
        int _serverFd;

        HttpServerContext get_ctx();

    public:
        HttpServer();
        ~HttpServer();
        HttpServer(const HttpServer& other) = delete;
        HttpServer& operator=(const HttpServer& other) = delete;

        std::future<HttpServerContext> get_ctx_async();
    };
}

#endif
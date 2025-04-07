#ifndef H_HTTP_SERVER
#define H_HTTP_SERVER

#include <future>
#include <vector>
#include <string>
#include <map>

#include "../libs/http-server-context.hpp"

// cas (Client and Server)
namespace cas 
{
    /// @brief Basically an HTTP Listener
    class HttpServer 
    {
        int _serverFd;
        int _port;

        HttpServerContext get_ctx();

    public:
        HttpServer();
        ~HttpServer();
        HttpServer(const HttpServer& other) = delete;
        HttpServer& operator=(const HttpServer& other) = delete;

        std::future<HttpServerContext> get_ctx_async();
        void set_port(const int port);
        void shutdown();
    };
}

#endif
/**
 * Christian Marcellino
 * 4/7/2025
 * 
 * This file is for our server http logic.
 */

#ifndef H_HTTP_SERVER
#define H_HTTP_SERVER

#include <future>
#include <vector>
#include <string>
#include <map>

#include "../libs/http-server-context.hpp"

#define DEFAULT_SERVER_BUFFER_SIZE 1024

// cas (Client and Server)
namespace cas 
{
    /// @brief Basically an HTTP Listener
    class HttpServer 
    {
        int _serverFd;
        int _port;
        int _bufferSize;

        HttpServerContext get_ctx();

    public:
        HttpServer(const int port, const int bufferSize);
        ~HttpServer();
        HttpServer(const HttpServer& other);
        HttpServer& operator=(const HttpServer& other);

        std::future<HttpServerContext> get_ctx_async();
        void set_port(const int port);
        void set_buffer_size(const int size);
        void shutdown();
    };
}

#endif
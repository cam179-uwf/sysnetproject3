/**
 * Christian Marcellino, Ryan Waddington
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
#include <vector>
#include <poll.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <functional>

#include "../libs/http-request.hpp"
#include "../libs/http-response.hpp"

#define DEFAULT_SERVER_BUFFER_SIZE 1024

namespace cas 
{
    struct HttpServerContext;

    /// @brief Basically an HTTP Listener
    class HttpServer
    {
        int _serverFd;
        int _port;
        int _bufferSize;
        std::vector<pollfd> _fds;
        std::map<int, std::string> _sessions;
        sockaddr_in _address;
        int _addrlen;

        /// @brief Gets an HTTP context for a single transaction. Blocks until a client request is available.
        /// @return The HttpServerContext.
        /// @throw ServerException
        HttpServerContext get_ctx();

        /// @brief Reads data from connected clients.
        /// @param clientFd 
        /// @param fdIndex 
        /// @param result 
        /// @return True if data was read, False otherwise.
        bool handleRead(int clientFd, size_t& fdIndex, HttpServerContext& result);

        /// @brief Accepts new client connections.
        void handleAccept();

    public:
        /// @brief A callback for capturing when a client connection is closed.
        std::function<void(int clientFd)> OnCloseClientConnection;

        HttpServer(const int port, const int bufferSize);
        ~HttpServer();
        HttpServer(const HttpServer& other) = default;
        HttpServer& operator=(const HttpServer& other) = default;

        /// @brief Gets an HTTP context for a single transaction. If awaited, blocks until a client request is available.
        /// @return An awaitable task that returns an HttpServerContext.
        /// @throw ServerException
        std::future<HttpServerContext> get_ctx_async();

        /// @brief Set the server's port.
        void set_port(const int port);

        /// @brief Set the server's buffer size for reading client requests.
        /// @param size The buffer size.
        void set_buffer_size(const int size);

        /// @brief Closes the server socket.
        void shutdown();

        /// @brief Check to see if a client file descriptor is closed. 
        bool is_client_connected(int clientFd);

        /// @brief Manually close a clients connection.
        void close_client_connection(int clientFd);
    };

    /// @brief For holding the context of a server HTTP transaction
    struct HttpServerContext
    {
        int fd = 0;
        HttpServer* server = nullptr;
        HttpRequest request;
        HttpResponse response;

        std::future<void> send_response_async();
        std::future<void> send_response_and_close_async();

    private:
        bool _wasSent = false;
    };
}

#endif
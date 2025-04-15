/**
 * Christian Marcellino
 * 4/7/2025
 * 
 * This file is for our server http context logic.
 * Used with the http-server.hpp.
 */

#ifndef H_HTTP_SERVER_CONTEXT
#define H_HTTP_SERVER_CONTEXT

#include <string>
#include <map>
#include <future>

#include "../libs/http-server-context.hpp"

namespace cas
{
    class HttpServer;

    /// @brief Handles client requests.
    struct HttpRequest
    {
        /// @brief Parse an HTTP header.
        /// @param content The raw HTTP header.
        void parse_header(const std::string& content);

        /// @brief Set the body of the HTTP request.
        void set_body(const std::string& body);

        /// @brief Gets the raw request as a string.
        /// @return The raw request.
        std::string to_string();

        /// @brief Gets the method of the HTTP request.
        std::string get_method() const;

        /// @brief Gets the path of the HTTP request.
        std::string get_path() const;

        /// @brief Gets the protocol of the HTTP request.
        std::string get_protocol() const;

        /// @brief Gets the headers of the HTTP request.
        std::map<std::string, std::string> get_headers() const;

        /// @brief Gets the body of the HTTP request.
        std::string get_body() const;

        /// @brief Determines if the specified key is in the header map.
        /// @param key The key to search.
        /// @return True if the key is found, otherwise false.
        bool contains_header(const std::string& key);

        /// @brief Determine if the specified key is in the header map, and if so, return its value.
        /// @param key The key to search.
        /// @param out The returned value.
        /// @return True if the key is found, otherwise false.
        bool try_get_header(const std::string& key, std::string& out);

    private:
        std::string _method;
        std::string _path;
        std::string _protocol;
        std::map<std::string, std::string> _headers;
        std::string _body;
    };

    /// @brief Handles server responses.
    struct HttpResponse
    {
        /// @brief An HTTP status.
        enum class Status 
        {
            Continue,
            SwitchingProtocols,
            OK,
            Created,
            Accepted,
            NoContent,
            MovedPermanently,
            Found,
            SeeOther,
            NotModified,
            TemporaryRedirect,
            PermanentRedirect,
            BadRequest,
            Unauthorized,
            Forbidden,
            NotFound,
            MethodNotAllowed,
            RequestTimeout,
            Conflict,
            LengthRequired,
            PayloadTooLarge,
            UnsupportedMediaType,
            UnprocessableEntity,
            TooManyRequests,
            InternalServerError,
            NotImplemented,
            BadGateway,
            ServiceUnavailable,
            GatewayTimeout
        };

        std::string protocol = "HTTP/1.1";
        size_t statusCode = 200;
        std::string statusMessage = "Success";
        std::map<std::string, std::string> headers;
        std::string body;

        /// @brief Sends a response to the client.
        /// @return An awaitable task.
        /// @throws ServerException
        std::future<void> sendoff_async();

        /// @brief Sends a response to the client then closes the connection.
        /// @return An awaitable task.
        /// @throws ServerException
        std::future<void> sendoff_close_async();

        /// @brief Gets the raw response as a string.
        /// @return The raw response.
        std::string to_string();

        /// @brief Set the HTTP status of the response.
        void set_status(const Status& status);

        /// @brief Should only be used internally.
        void __set_server(HttpServer& server);

        /// @brief Should only be used internally.
        void __set_client_fd(const int clientFd);

        /// @brief Should only be used internally.
        int __get_client_fd() const;
        
    private:
        int _clientFd = 0;
        HttpServer* _server;
        bool _wasSent = false;
    };

    /// @brief For holding the context of a server HTTP transaction
    struct HttpServerContext
    {
        HttpRequest request;
        HttpResponse response;
    };
}

#endif
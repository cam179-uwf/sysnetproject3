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

#include "../libs/http-server-context.hpp"

namespace cas
{
    class HttpServer;

    /// @brief For keeping track of requests
    struct HttpRequest
    {
        void parse(const std::string& content);
        std::string to_string();

        std::string get_method() const;
        std::string get_path() const;
        std::string get_protocol() const;
        std::map<std::string, std::string> get_headers() const;
        std::string get_body() const;

        bool contains_header(const std::string& key);
        bool try_get_header(const std::string& key, std::string& out);

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

        std::future<void> sendoff_async();
        std::future<void> sendoff_close_async();
        std::string to_string();

        void set_status(const Status& status);

        void __set_server(HttpServer& server);
        void __set_client_fd(const int clientFd);
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
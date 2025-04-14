/**
 * Christian Marcellino
 * 4/7/2025
 * 
 * This file is for our server http context logic.
 */

#include "../libs/http-server-context.hpp"

#include <iostream>
#include <sstream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "../libs/string-helpers.hpp"
#include "../libs/exceptions.hpp"
#include "../libs/http-server.hpp"

using namespace cas;

/// @return The HTTP method.
std::string cas::HttpRequest::get_method() const
{
    return _method;
}

/// @return The requested path.
std::string cas::HttpRequest::get_path() const
{
    return _path;
}

/// @return The HTTP protocol.
std::string cas::HttpRequest::get_protocol() const
{
    return _protocol;
}

/// @return The HTTP headers.
std::map<std::string, std::string> cas::HttpRequest::get_headers() const
{
    return _headers;
}

/// @return The HTTP body.
std::string cas::HttpRequest::get_body() const
{
    return _body;
}

bool cas::HttpRequest::contains_header(const std::string& key)
{
    return _headers.find(key) != _headers.end();
}

bool cas::HttpRequest::try_get_header(const std::string& key, std::string &out)
{
    if (contains_header(key))
    {
        out = _headers[key];
        return true;
    }
    return false;
}

/// @brief Parses a raw HTTP request.
/// @param content The raw HTTP request.
void cas::HttpRequest::parse_header(const std::string& content)
{
    std::istringstream iss(content);

    std::string firstLine;
    std::getline(iss, firstLine);

    auto firstLineSplits = strhelp::split(firstLine, ' ', 3);

    if (firstLineSplits.size() >= 1)
    {
        _method = firstLineSplits[0];
    }

    if (firstLineSplits.size() >= 2)
    {
        _path = firstLineSplits[1];
    }

    if (firstLineSplits.size() >= 3)
    {
        _protocol = firstLineSplits[2];
    }

    std::string header;
    std::getline(iss, header);

    while (header.size() > 0 && !iswspace(header[0]))
    {
        auto headerSplits = strhelp::split(header, ':');

        if (headerSplits.size() == 2)
        {
            _headers[headerSplits[0]] = strhelp::trim(headerSplits[1]);
        }

        std::getline(iss, header);
    }
}

void cas::HttpRequest::set_body(const std::string &body)
{
    _body = body;
}

/// @return The raw HTTP request.
std::string cas::HttpRequest::to_string()
{
    std::ostringstream oss;

    oss << _method << " " << _path << " " << _protocol << std::endl;

    for (auto header : _headers)
    {
        oss << header.first << ": " << header.second << std::endl;
    }

    oss << std::endl;
    oss << _body;

    return oss.str();
}

/// @brief Sends the HTTP response and closes this clients socket.
/// @return A promise.
std::future<void> cas::HttpResponse::sendoff_async()
{
    if (_wasSent) throw std::runtime_error("[sendoff_async] Policy dictates that their can only be one response sent per context.");
    _wasSent = true;
    
    return std::async([this]() {
        // ==================
        // | Build Response |
        // ==================
        std::ostringstream oss;
        headers["Content-Length"] = std::to_string(body.size());

        oss << protocol << " " << statusCode << " " << statusMessage << "\r\n";

        for (auto header : headers)
        {
            oss << header.first << ": " << header.second << "\r\n";
        }

        oss << "\r\n";
        oss << body;

        std::string content(oss.str());

        if (VERBOSE_DEBUG)
        {
            std::cout << "Sending message to clientFd: " << _clientFd << std::endl;
        }

        // send the response to the client
        if (send(_clientFd, content.c_str(), content.size(), 0) < 0)
        {
            switch (errno)
            {
            case EAGAIN | EWOULDBLOCK: throw ServerException("Failed to send message to client: the socket's file descriptor is marked O_NONBLOCK and the requested operation would block.");
            case EBADF: throw ServerException("Failed to send message to client: the socket argument is not a valid file descriptor.");
            case ECONNRESET: throw ServerException("Failed to send message to client: a connection was forcibly closed by a peer.");
            case EDESTADDRREQ: throw ServerException("Failed to send message to client: the socket is not connection-mode and no peer address is set.");
            case EINTR: throw ServerException("Failed to send message to client: a signal interrupted send() before any data was transmitted.");
            case EMSGSIZE: throw ServerException("Failed to send message to client: the message is too large to be sent all at once, as the socket requires.");
            case ENOTCONN: throw ServerException("Failed to send message to client: the socket is not connected.");
            case ENOTSOCK: throw ServerException("Failed to send message to client: the socket argument does not refer to a socket.");
            case EOPNOTSUPP: throw ServerException("Failed to send message to client: the socket argument is associated with a socket that does not support one or more of the values set in flags.");
            case EPIPE: throw ServerException("Failed to send message to client: the socket is shut down for writing, or the socket is connection-mode and is no longer connected. In the latter case, and if the socket is of type SOCK_STREAM or SOCK_SEQPACKET and the MSG_NOSIGNAL flag is not set, the SIGPIPE signal is generated to the calling thread.");
            case EACCES: throw ServerException("Failed to send message to client: the calling process does not have appropriate privileges.");
            case EIO: throw ServerException("Failed to send message to client: an I/O error occurred while reading from or writing to the file system.");
            case ENETDOWN: throw ServerException("Failed to send message to client: the local network interface used to reach the destination is down.");
            case ENETUNREACH: throw ServerException("Failed to send message to client: no route to the network is present.");
            case ENOBUFS: throw ServerException("Failed to send message to client: insufficient resources were available in the system to perform the operation.");
            default: throw ServerException("Failed to send message to client.");
            }
        }
    });
}

std::future<void> cas::HttpResponse::sendoff_close_async()
{
    if (_wasSent) throw std::runtime_error("[sendoff_close_async] Policy dictates that their can only be one response sent per context.");

    return std::async([this]() {
        sendoff_async().get();

        // WARNING: this could fail if the pointer is dereferenced before this is called
        _server->close_client_connection(_clientFd);
    });
}

/// @return The raw HTTP response.
std::string cas::HttpResponse::to_string()
{
    std::ostringstream oss;

    oss << protocol << " " << statusCode << " " << statusMessage << std::endl;

    for (auto header : headers)
    {
        oss << header.first << ": " << header.second << std::endl;
    }

    oss << std::endl;
    oss << body;

    return oss.str();
}

void cas::HttpResponse::set_status(const Status &status)
{
    switch (status)
    {
    case HttpResponse::Status::Continue:
        statusCode = 100;
        statusMessage = "Continue";
        break;
    case HttpResponse::Status::SwitchingProtocols: 
        statusCode = 101;
        statusMessage = "Switching Protocols";
        break;
    case HttpResponse::Status::OK:
        statusCode = 200;
        statusMessage = "OK";
        break;
    case HttpResponse::Status::Created:
        statusCode = 201;
        statusMessage = "Created";
        break;
    case HttpResponse::Status::Accepted:
        statusCode = 202;
        statusMessage = "Accepted";
        break;
    case HttpResponse::Status::NoContent:
        statusCode = 204;
        statusMessage = "No Content";
        break;
    case HttpResponse::Status::MovedPermanently:
        statusCode = 301;
        statusMessage = "Moved Permanently";
        break;
    case HttpResponse::Status::Found:
        statusCode = 302;
        statusMessage = "Found";
        break;
    case HttpResponse::Status::SeeOther:
        statusCode = 303;
        statusMessage = "See Other";
        break;
    case HttpResponse::Status::NotModified:
        statusCode = 304;
        statusMessage = "Not Modified";
        break;
    case HttpResponse::Status::TemporaryRedirect:
        statusCode = 307;
        statusMessage = "Temporary Redirect";
        break;
    case HttpResponse::Status::PermanentRedirect:
        statusCode = 308;
        statusMessage = "Permanent Redirect";
        break;
    case HttpResponse::Status::BadRequest:
        statusCode = 400;
        statusMessage = "Bad Request";
        break;
    case HttpResponse::Status::Unauthorized:
        statusCode = 401;
        statusMessage = "Unauthorized";
        break;
    case HttpResponse::Status::Forbidden:
        statusCode = 403;
        statusMessage = "Forbidden";
        break;
    case HttpResponse::Status::NotFound:
        statusCode = 404;
        statusMessage = "Not Found";
        break;
    case HttpResponse::Status::MethodNotAllowed:
        statusCode = 405;
        statusMessage = "Method Not Allowed";
        break;
    case HttpResponse::Status::RequestTimeout:
        statusCode = 408;
        statusMessage = "Request Timeout";
        break;
    case HttpResponse::Status::Conflict:
        statusCode = 409;
        statusMessage = "Conflict";
        break;
    case HttpResponse::Status::LengthRequired:
        statusCode = 411;
        statusMessage = "Length Required";
        break;
    case HttpResponse::Status::PayloadTooLarge:
        statusCode = 413;
        statusMessage = "Payload Too Large";
        break;
    case HttpResponse::Status::UnsupportedMediaType:
        statusCode = 415;
        statusMessage = "Unsupported Media Type";
        break;
    case HttpResponse::Status::UnprocessableEntity:
        statusCode = 422;
        statusMessage = "Unprocessable Entity";
        break;
    case HttpResponse::Status::TooManyRequests:
        statusCode = 429;
        statusMessage = "Too Many Requests";
        break;
    case HttpResponse::Status::InternalServerError:
        statusCode = 500;
        statusMessage = "Internal Server Error";
        break;
    case HttpResponse::Status::NotImplemented:
        statusCode = 501;
        statusMessage = "Not Implemented";
        break;
    case HttpResponse::Status::BadGateway:
        statusCode = 502;
        statusMessage = "Bad Gateway";
        break;
    case HttpResponse::Status::ServiceUnavailable:
        statusCode = 503;
        statusMessage = "Service Unavailable";
        break;
    case HttpResponse::Status::GatewayTimeout:
        statusCode = 504;
        statusMessage = "Gateway Timeout";
        break;
    }
}

void cas::HttpResponse::__set_server(HttpServer& server)
{
    _server = &server;
}

void cas::HttpResponse::__set_client_fd(const int clientFd)
{
    _clientFd = clientFd;
}

int cas::HttpResponse::__get_client_fd() const
{
    return _clientFd;
}

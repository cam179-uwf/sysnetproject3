#include "../libs/http-server.hpp"

#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sstream>

#include "../libs/http-server.hpp"
#include "../libs/string-helpers.hpp"

using namespace cas;

// std::vector<std::string> split(std::string text, char delimiter)
// {
//     std::vector<std::string> result;
//     size_t start = 0, end = 0;

//     for (size_t i = 0; i < text.size(); ++i)
//     {
//         if (text[i] == delimiter)
//         {
//             end = i;

//             result.push_back(text.substr(start, end - start));

//             start = i;
//         }
//     }

//     return result;
// }

// ==============
// | HttpServer |
// ==============

HttpServer::HttpServer()
{
    _serverFd = 0;
}

HttpServer::~HttpServer()
{
    close(_serverFd);
}

HttpServerContext HttpServer::get_ctx()
{
    HttpServerContext result;

    int client_fd;
    sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    if (_serverFd <= 0)
    {
        _serverFd = socket(AF_INET, SOCK_STREAM, 0);

        if (_serverFd <= 0)
        {
            switch (errno)
            {
            case EAFNOSUPPORT: throw ServerException("Server failed to create a socket: the implementation does not support the specified address family.");
            case EMFILE: throw ServerException("Server failed to create a socket: no more file descriptors are available for this process.");
            case ENFILE: throw ServerException("Server failed to create a socket: no more file descriptors are available for the system.");
            case EPROTONOSUPPORT: throw ServerException("Server failed to create a socket: the protocol is not supported by the address family, or the protocol is not supported by the implementation.");
            case EPROTOTYPE: throw ServerException("Server failed to create a socket: the socket type is not supported by the protocol.");
            case EACCES: throw ServerException("Server failed to create a socket: the process does not have appropriate privileges.");
            case ENOBUFS: throw ServerException("Server failed to create a socket: insufficient resources were available in the system to perform the operation.");
            case ENOMEM: throw ServerException("Server failed to create a socket: insufficient memory was available to fulfill the request.");
            default: throw ServerException("Server failed to create a socket.");
            }        
        }

        setsockopt(_serverFd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY; // 0.0.0.0
        address.sin_port = htons(8080);

        if (bind(_serverFd, (sockaddr*)&address, addrlen) < 0)
        {
            switch (errno)
            {
            case EADDRINUSE: throw ServerException("Server failed to bind the socket: the specified address is already in use.");
            case EADDRNOTAVAIL: throw ServerException("Server failed to bind the socket: the specified address is not available from the local machine.");
            case EAFNOSUPPORT: throw ServerException("Server failed to bind the socket: the specified address is not a valid address for the address family of the specified socket.");
            case EALREADY: throw ServerException("Server failed to bind the socket: an assignment request is already in progress for the specified socket.");
            case EBADF: throw ServerException("Server failed to bind the socket: the socket argument is not a valid file descriptor.");
            case EINPROGRESS: throw ServerException("Server failed to bind the socket: O_NONBLOCK is set for the file descriptor for the socket and the assignment cannot be immediately performed; the assignment shall be performed asynchronously.");
            case EINVAL: throw ServerException("Server failed to bind the socket: the socket is already bound to an address, and the protocol does not support binding to a new address; or the socket has been shut down. Or, the address_len argument is not a valid length for the address family.");
            case ENOBUFS: throw ServerException("Server failed to bind the socket: insufficient resources were available to complete the call.");
            case ENOTSOCK: throw ServerException("Server failed to bind the socket: the socket argument does not refer to a socket.");
            case EOPNOTSUPP: throw ServerException("Server failed to bind the socket: the socket type of the specified socket does not support binding to an address.");

            // IF AF_UNIX
            // case EACCES: throw ServerException("Server failed to bind the socket: a component of the path prefix denies search permission, or the requested name requires writing in a directory with a mode that denies write permission.");
            // case EDESTADDRREQ: throw ServerException("Server failed to bind the socket: the address argument is a null pointer.");
            // case EISDIR: throw ServerException("Server failed to bind the socket: the address argument is a null pointer.");
            // case EIO: throw ServerException("Server failed to bind the socket: an I/O error occurred.");
            // case ELOOP: throw ServerException("Server failed to bind the socket: a loop exists in symbolic links encountered during resolution of the pathname in address.");
            // case ENAMETOOLONG: throw ServerException("Server failed to bind the socket: the length of a component of a pathname is longer than {NAME_MAX}.");
            // case ENOENT: throw ServerException("Server failed to bind the socket: The pathname in address contains at least one non- <slash> character and ends with one or more trailing <slash> characters. If the pathname without the trailing <slash> characters would name an existing file, an [ENOENT] error shall not occur.");
            // case ENOTDIR: throw ServerException("Server failed to bind the socket: The pathname in address contains at least one non- <slash> character and ends with one or more trailing <slash> characters. If the pathname without the trailing <slash> characters would name an existing file, an [ENOENT] error shall not occur.");
            // case EROFS: throw ServerException("Server failed to bind the socket: the name would reside on a read-only file system.");
            
            case EACCES: throw ServerException("Server failed to bind the socket: the specified address is protected and the current user does not have permission to bind to it.");
            case EISCONN: throw ServerException("Server failed to bind the socket: the socket is already connected.");
            case ELOOP: throw ServerException("Server failed to bind the socket: more than {SYMLOOP_MAX} symbolic links were encountered during resolution of the pathname in address.");
            case ENAMETOOLONG: throw ServerException("Server failed to bind the socket: the length of a pathname exceeds {PATH_MAX}, or pathname resolution of a symbolic link produced an intermediate result with a length that exceeds {PATH_MAX}.");

            default: throw ServerException("Server failed to bind the socket."); 
            }
        }
    }

    if (listen(_serverFd, 3) < 0)
    {
        switch (errno)
        {
        case EBADF: throw ServerException("Server failed to start listening: the socket argument is not a valid file descriptor.");
        case EDESTADDRREQ: throw ServerException("Server failed to start listening: the socket is not bound to a local address, and the protocol does not support listening on an unbound socket.");
        case EINVAL: throw ServerException("Server failed to start listening: the socket is already connected. Or, the socket has been shut down.");
        case ENOTSOCK: throw ServerException("Server failed to start listening: the socket argument does not refer to a socket.");
        case EOPNOTSUPP: throw ServerException("Server failed to start listening: the socket protocol does not support listen().");
        case EACCES: throw ServerException("Server failed to start listening: the calling process does not have appropriate privileges.");
        case ENOBUFS: throw ServerException("Server failed to start listening: insufficient resources are available in the system to complete the call.");
        default: throw ServerException("Server failed to start listening.");
        }        
    }

    client_fd = accept(_serverFd, (sockaddr*)&address, (socklen_t*)&addrlen);

    if (client_fd < 0)
    {
        switch (errno)
        {
        case EAGAIN | EWOULDBLOCK: throw ServerException("Failed to accept: no connections are present.");
        case EBADF: throw ServerException("Failed to accept: the socket argument is not a valid file descriptor.");
        case ECONNABORTED: throw ServerException("Failed to accept: the connection has been aborted.");
        case EINTR: throw ServerException("Failed to accept: interrupted by a signal that was caught before a valid connection arrived.");
        case EINVAL: throw ServerException("Failed to accept: the socket is not accepting connections.");
        case EMFILE: throw ServerException("Failed to accept: " + std::to_string(_SC_OPEN_MAX) + " file descriptors are currently open in the calling process.");
        case ENFILE: throw ServerException("Failed to accept: the maximum number of file descriptors in the system are already open.");
        case ENOTSOCK: throw ServerException("Failed to accept: the socket argument does not refer to a socket.");
        case EOPNOTSUPP: throw ServerException("Failed to accept: the socket type of the specified socket does not support accepting connections.");
        case ENOBUFS: throw ServerException("Failed to accept: no buffer space is available.");
        case ENOMEM: throw ServerException("Failed to accept: there was insufficient memory available to complete the operation.");
        case EPROTO: throw ServerException("Failed to accept: a protocol error has occurred; for example, the STREAMS protocol stack has not been initialized.");
        default: throw ServerException("Failed to accept an incoming message. (" + std::to_string(client_fd) + ")");
        }
    }

    char buffer[1024] = {0};
    read(client_fd, buffer, 1024);
    
    result.get_req().parse(buffer, 1024);
    
    // std::string response = "Hello from server!";
    // send(client_fd, response.c_str(), response.size(), 0);

    return result;
}

std::future<HttpServerContext> HttpServer::get_ctx_async()
{
    return std::async(std::launch::async, &HttpServer::get_ctx, this);
}

// =====================
// | HttpServerContext |
// =====================

cas::HttpServerContext::HttpServerContext()
{ }

cas::HttpServerContext::~HttpServerContext()
{ }

cas::HttpServerContext::HttpServerContext(const HttpServerContext& other)
{
    _request = other._request;
    _response = other._response;
}

HttpServerContext& cas::HttpServerContext::operator=(const HttpServerContext& other)
{
    _request = other._request;
    _response = other._response;
    return *this;
}

HttpRequest& cas::HttpServerContext::get_req()
{
    return _request;
}

HttpResponse& cas::HttpServerContext::get_res()
{
    return _response;
}

std::string cas::HttpRequest::get_method() const
{
    return _method;
}

std::string cas::HttpRequest::get_path() const
{
    return _path;
}

std::string cas::HttpRequest::get_protocol() const
{
    return _protocol;
}

std::map<std::string, std::string> cas::HttpRequest::get_headers() const
{
    return _headers;
}

std::string cas::HttpRequest::get_body() const
{
    return _body;
}

void cas::HttpRequest::parse(const char *buffer, size_t length)
{
    std::string content(buffer, length);

    std::istringstream iss(content);

    std::string firstLine;
    std::getline(iss, firstLine);

    auto firstLineSplits = strhelp::split(firstLine, ' ');

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
            _headers[headerSplits[0]] = headerSplits[1];
        }

        std::getline(iss, header);
    }

    std::string bodyLine;
    std::ostringstream bodyBuilder;

    while (std::getline(iss, bodyLine))
    {
        bodyBuilder << bodyLine;
    }

    _body = bodyBuilder.str();
}

void cas::HttpResponse::set_client_fd(int client_fd)
{
    _clientFd = client_fd;
}

void cas::HttpResponse::send_and_close()
{
    close(_clientFd);
}

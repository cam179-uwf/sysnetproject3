#include "../libs/http-client.hpp"

#include <sstream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>

#include "../libs/exceptions.hpp"
#include "../libs/string-helpers.hpp"

using namespace cas;

std::string build_http_request_str(const std::string& method, const HttpClientRequest& request)
{
    std::ostringstream oss;

    oss << method << " " << request.path << " HTTP/1.1" << std::endl;

    for (auto header : request.headers)
    {
        oss << header.first << ": " << header.second << std::endl;
    }

    if (request.headers.size() <= 0)
    {
        oss << std::endl;
    }

    oss << request.body;

    return oss.str();
}

HttpClientResponse parse(const std::string& response)
{
    HttpClientResponse res;

    std::istringstream iss(response);

    std::string firstLine;
    std::getline(iss, firstLine);

    auto firstLineSplits = strhelp::split(firstLine, ' ');

    if (firstLineSplits.size() >= 1)
    {
        res.httpVersion = firstLineSplits[0];
    }

    if (firstLineSplits.size() >= 2)
    {
        try 
        {
            res.statusCode = std::stoi(firstLineSplits[1]);
        }
        catch (const std::exception& ex)
        {
            // TODO: probably shouldn't throw lol
            throw ClientException(ex.what());
        }
    }

    if (firstLineSplits.size() >= 3)
    {
        res.statusMessage = firstLineSplits[2];
    }

    std::string header;
    std::getline(iss, header);

    while (header.size() > 0 && !iswspace(header[0]))
    {
        auto headerSplits = strhelp::split(header, ':');

        if (headerSplits.size() == 2)
        {
            res.headers[headerSplits[0]] = strhelp::trim(headerSplits[1]);
        }

        std::getline(iss, header);
    }

    std::string bodyLine;
    std::ostringstream bodyBuilder;

    while (std::getline(iss, bodyLine))
    {
        bodyBuilder << bodyLine;
    }

    res.body = bodyBuilder.str();
    
    return res;
}

HttpClientResponse HttpClient::make_request(const std::string& method, const HttpClientRequest& request)
{
    int clientFd = socket(AF_INET, SOCK_STREAM, 0);

    if (clientFd < 0)
    {
        switch (errno)
        {
        case EAFNOSUPPORT: throw ClientException("Client failed to create a socket: the implementation does not support the specified address family.");
        case EMFILE: throw ClientException("Client failed to create a socket: no more file descriptors are available for this process.");
        case ENFILE: throw ClientException("Client failed to create a socket: no more file descriptors are available for the system.");
        case EPROTONOSUPPORT: throw ClientException("Client failed to create a socket: the protocol is not supported by the address family, or the protocol is not supported by the implementation.");
        case EPROTOTYPE: throw ClientException("Client failed to create a socket: the socket type is not supported by the protocol.");
        case EACCES: throw ClientException("Client failed to create a socket: the process does not have appropriate privileges.");
        case ENOBUFS: throw ClientException("Client failed to create a socket: insufficient resources were available in the system to perform the operation.");
        case ENOMEM: throw ClientException("Client failed to create a socket: insufficient memory was available to fulfill the request.");
        default: throw ClientException("Client failed to create a socket.");
        }
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(_port);

    int inetPtonResult = inet_pton(AF_INET, _host.c_str(), &serverAddr.sin_addr);

    if (inetPtonResult < 0)
    {
        close(clientFd);

        switch (errno)
        {
        case EAFNOSUPPORT: throw ClientException("Failed to convert address: the address family specified in af is unsupported.");
        default: throw ClientException("Failed to convert address.");
        }
    }
    else if (inetPtonResult == 0)
    {
        close(clientFd);

        throw ClientException("Failed to convert address: unsuccessful because the input buffer pointed to by src is not a valid string.");
    }

    if (connect(clientFd, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
    {
        close(clientFd);
        
        switch (errno)
        {
        case EADDRNOTAVAIL: throw ClientException("Failed to connect: the specified address is not available from the local machine.");
        case EAFNOSUPPORT: throw ClientException("Failed to connect: the specified address is not a valid address for the address family of the specified socket.");
        case EALREADY: throw ClientException("Failed to connect: a connection request is already in progress for the specified socket.");
        case EBADF: throw ClientException("Failed to connect: the socket argument is not a valid file descriptor.");
        case ECONNREFUSED: throw ClientException("Failed to connect: the target address was not listening for connections or refused the connection request.");
        case EINPROGRESS: throw ClientException("Failed to connect: O_NONBLOCK is set for the file descriptor for the socket and the connection cannot be immediately established; the connection shall be established asynchronously.");
        case EINTR: throw ClientException("Failed to connect: the attempt to establish a connection was interrupted by delivery of a signal that was caught; the connection shall be established asynchronously.");
        case EISCONN: throw ClientException("Failed to connect: the specified socket is connection-mode and is already connected.");
        case ENETUNREACH: throw ClientException("Failed to connect: no route to the network is present.");
        case ENOTSOCK: throw ClientException("Failed to connect: the socket argument does not refer to a socket.");
        case EPROTOTYPE: throw ClientException("Failed to connect: the specified address has a different type than the socket bound to the specified peer address.");
        case ETIMEDOUT: throw ClientException("Failed to connect: the attempt to connect timed out before a connection was made.");

        // AF_UNIX
        // case EIO: throw ClientException("Failed to connect: an I/O error occurred while reading from or writing to the file system.");
        // case ELOOP: throw ClientException("Failed to connect: a loop exists in symbolic links encountered during resolution of the pathname in address.");
        // case ENAMETOOLONG: throw ClientException("Failed to connect: the length of a component of a pathname is longer than {NAME_MAX}.");
        // case ENOENT: throw ClientException("Failed to connect: a component of the pathname does not name an existing file or the pathname is an empty string.");
        // case ENOTDIR: throw ClientException("Failed to connect: a component of the path prefix of the pathname in address names an existing file that is neither a directory nor a symbolic link to a directory, or the pathname in address contains at least one non- <slash> character and ends with one or more trailing <slash> characters and the last pathname component names an existing file that is neither a directory nor a symbolic link to a directory.");

        case EACCES: throw ClientException("Failed to connect: search permission is denied for a component of the path prefix; or write access to the named socket is denied.");
        case EADDRINUSE: throw ClientException("Failed to connect: attempt to establish a connection that uses addresses that are already in use.");
        case ECONNRESET: throw ClientException("Failed to connect: remote host reset the connection request.");
        case EHOSTUNREACH: throw ClientException("Failed to connect: the destination host cannot be reached (probably because the host is down or a remote router cannot reach it).");
        case EINVAL: throw ClientException("Failed to connect: the address_len argument is not a valid length for the address family; or invalid address family in the sockaddr structure.");
        case ELOOP: throw ClientException("Failed to connect: more than {SYMLOOP_MAX} symbolic links were encountered during resolution of the pathname in address.");
        case ENAMETOOLONG: throw ClientException("Failed to connect: the length of a pathname exceeds {PATH_MAX}, or pathname resolution of a symbolic link produced an intermediate result with a length that exceeds {PATH_MAX}.");
        case ENETDOWN: throw ClientException("Failed to connect: the local network interface used to reach the destination is down.");
        case ENOBUFS: throw ClientException("Failed to connect: no buffer space is available.");
        case EOPNOTSUPP: throw ClientException("Failed to connect: the socket is listening and cannot be connected.");
        default: throw ClientException("Failed to connect.");
        }
    }

    std::string content = build_http_request_str(method, request);

    if (send(clientFd, content.c_str(), content.size(), 0) < 0)
    {
        close(clientFd);
        
        switch (errno)
        {
        case EAGAIN | EWOULDBLOCK: throw ServerException("Failed to send message to server: the socket's file descriptor is marked O_NONBLOCK and the requested operation would block.");
        case EBADF: throw ServerException("Failed to send message to server: the socket argument is not a valid file descriptor.");
        case ECONNRESET: throw ServerException("Failed to send message to server: a connection was forcibly closed by a peer.");
        case EDESTADDRREQ: throw ServerException("Failed to send message to server: the socket is not connection-mode and no peer address is set.");
        case EINTR: throw ServerException("Failed to send message to server: a signal interrupted send() before any data was transmitted.");
        case EMSGSIZE: throw ServerException("Failed to send message to server: the message is too large to be sent all at once, as the socket requires.");
        case ENOTCONN: throw ServerException("Failed to send message to server: the socket is not connected.");
        case ENOTSOCK: throw ServerException("Failed to send message to server: the socket argument does not refer to a socket.");
        case EOPNOTSUPP: throw ServerException("Failed to send message to server: the socket argument is associated with a socket that does not support one or more of the values set in flags.");
        case EPIPE: throw ServerException("Failed to send message to server: the socket is shut down for writing, or the socket is connection-mode and is no longer connected. In the latter case, and if the socket is of type SOCK_STREAM or SOCK_SEQPACKET and the MSG_NOSIGNAL flag is not set, the SIGPIPE signal is generated to the calling thread.");
        case EACCES: throw ServerException("Failed to send message to server: the calling process does not have appropriate privileges.");
        case EIO: throw ServerException("Failed to send message to server: an I/O error occurred while reading from or writing to the file system.");
        case ENETDOWN: throw ServerException("Failed to send message to server: the local network interface used to reach the destination is down.");
        case ENETUNREACH: throw ServerException("Failed to send message to server: no route to the network is present.");
        case ENOBUFS: throw ServerException("Failed to send message to server: insufficient resources were available in the system to perform the operation.");
        default: throw ServerException("Failed to send message to server.");
        }
    }

    std::ostringstream oss;

    char buffer[_bufferSize];
    ssize_t n;

    while ((n = read(clientFd, buffer, _bufferSize - 1)) > 0)
    {
        if (n < 0)
        {
            switch (errno)
            {
            case EAGAIN: throw ServerException("Failed to read: the file is a pipe or FIFO, the O_NONBLOCK flag is set for the file descriptor, and the thread would be delayed in the read operation.");
            case EBADF: throw ServerException("Failed to read: the fildes argument is not a valid file descriptor open for reading.");
            case EBADMSG: throw ServerException("Failed to read: the file is a STREAM file that is set to control-normal mode and the message waiting to be read includes a control part.");
            case EINTR: throw ServerException("Failed to read: the read operation was terminated due to the receipt of a signal, and no data was transferred.");
            case EINVAL: throw ServerException("Failed to read: the STREAM or multiplexer referenced by fildes is linked (directly or indirectly) downstream from a multiplexer.");
            case EIO: throw ServerException("Failed to read: the process is a member of a background process group attempting to read from its controlling terminal, and either the calling thread is blocking SIGTTIN or the process is ignoring SIGTTIN or the process group of the process is orphaned. This error may also be generated for implementation-defined reasons.");
            case EISDIR: throw ServerException("Failed to read: the fildes argument refers to a directory and the implementation does not allow the directory to be read using read() or pread(). The readdir() function should be used instead.");
            case EOVERFLOW: throw ServerException("Failed to read: the file is a regular file, nbyte is greater than 0, the starting position is before the end-of-file, and the starting position is greater than or equal to the offset maximum established in the open file description associated with fildes.");
            case ECONNRESET: throw ServerException("Failed to read: a read was attempted on a socket and the connection was forcibly closed by its peer.");
            case ENOTCONN: throw ServerException("Failed to read: a read was attempted on a socket that is not connected.");
            case ETIMEDOUT: throw ServerException("Failed to read: a read was attempted on a socket and a transmission timeout occurred.");
            case ENOBUFS: throw ServerException("Failed to read: insufficient resources were available in the system to perform the operation.");
            case ENOMEM: throw ServerException("Failed to read: insufficient memory was available to fulfill the request.");
            case ENXIO: throw ServerException("Failed to read: a request was made of a nonexistent device, or the request was outside the capabilities of the device.");
            default: throw ServerException("Failed to read.");
            }
        }

        buffer[n] = '\0';
        oss << buffer;
    }

    close(clientFd);

    return parse(oss.str());
}

HttpClient::HttpClient(const std::string& host, const int port, const int bufferSize)
{
    _host = host;
    _port = port;
    _bufferSize = bufferSize;
}

HttpClient::~HttpClient()
{
    
}

HttpClient::HttpClient(const HttpClient& other)
{
    _host = other._host;
    _port = other._port;
}

HttpClient& HttpClient::operator=(const HttpClient& other)
{
    if (this != &other)
    {
        _host = other._host;
        _port = other._port;
    }
    return *this;
}

std::future<HttpClientResponse> HttpClient::get_async(const HttpClientRequest& request)
{
    return std::async(&HttpClient::make_request, this, "GET", request);
}

std::future<HttpClientResponse> HttpClient::post_async(const HttpClientRequest& request)
{
    return std::async(&HttpClient::make_request, this, "POST", request);
}

std::string cas::HttpClientResponse::to_string()
{
    std::ostringstream oss;

    oss << "HTTP/1.1 " << statusCode << " " << statusMessage << std::endl;

    for (auto header : headers)
    {
        oss << header.first << ": " << header.second << std::endl;
    }

    if (headers.size() <= 0)
    {
        oss << std::endl;
    }

    oss << body;

    return oss.str();
}

std::string cas::HttpClientRequest::to_string()
{
    std::ostringstream oss;

    oss << "[TBD] " << path << " HTTP/1.1" << std::endl;

    for (auto header : headers)
    {
        oss << header.first << ": " << header.second << std::endl;
    }

    if (headers.size() <= 0)
    {
        oss << std::endl;
    }

    oss << body;

    return oss.str();
}

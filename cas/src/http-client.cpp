/**
 * Christian Marcellino
 * 4/7/2025
 * 
 * This file is for our client http logic.
 */

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

HttpClient::HttpClient(const std::string& host, const int port, const int bufferSize)
{
    _host = host;
    _port = port;
    _bufferSize = bufferSize;
}

HttpClient::~HttpClient()
{
    
}

/// @brief Builds a string that becomes the raw HTTP request that is sent over the network.
/// @param method The method to use for the HTTP request.
/// @param request The HTTP request.
/// @return A string representing the raw HTTP request.
std::string build_http_request_str(const std::string& method, const HttpClientRequest& request)
{
    std::map<std::string, std::string> headers = request.headers;
    headers["Content-Length"] = std::to_string(request.body.size());

    std::ostringstream oss;

    oss << method << " " << request.path << " HTTP/1.1" << "\r\n";

    for (auto header : headers)
    {
        oss << header.first << ": " << header.second << "\r\n";
    }

    oss << "\r\n";
    oss << request.body;

    return oss.str();
}

/// @brief Parses an HTTP response.
/// @param response The raw HTTP response to parse.
/// @return An HttpClientResponse.
HttpClientResponse parse_header(const std::string& response)
{
    HttpClientResponse res;

    std::istringstream iss(response);

    std::string firstLine;
    std::getline(iss, firstLine);

    auto firstLineSplits = strhelp::split(firstLine, ' ', 3);

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
            res.statusCode = -1;
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
    
    return res;
}

/// @brief Makes an HTTP request using the provided method and request.
/// @param method The HTTP method to use.
/// @param request The HTTP request.
/// @return A HttpClientResponse.
/// @throw ClientException
HttpClientResponse HttpClient::make_request(const std::string& method, const HttpClientRequest& request)
{
    // open a socket for the client
    int clientFd = socket(AF_INET, SOCK_STREAM, 0); // use IPv4 and TCP

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
    serverAddr.sin_family = AF_INET; // use IPv4
    serverAddr.sin_port = htons(_port); // set the port number

    // convert from presentation format of an Internet number in buffer
    // starting at CP to the binary network format and store result for
    // interface type AF in buffer starting at BUF.
    // 
    // stores the result in serverAddr.sin_addr
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

    // try to connect to the server
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

    // send request to the server
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
    int seqCheck = 0;
    FdReader reader(clientFd, _bufferSize);

    // read response from the server not limited to _bufferSize
    while (reader.still_connected())
    {
        char c = reader.read_next();

        if (c == '\r' && seqCheck == 0)
        {
            seqCheck = 1;
        }
        else if (c == '\n' && seqCheck == 1)
        {
            seqCheck = 2;
        }
        else if (c == '\r' && seqCheck == 2)
        {
            seqCheck = 3;
        }
        else if (c == '\n' && seqCheck == 3)
        {
            break;
        }
        else
        {
            seqCheck = 0;
        }

        oss << c;
    }

    auto response = parse_header(oss.str());
    
    if (response.headers.find("Content-Length") != response.headers.end())
    {
        try
        {
            int contentLength = std::stoi(response.headers["Content-Length"]);

            if (VERBOSE_DEBUG)
            {
                std::cout << "Read body of content length: " << contentLength << std::endl;
            }

            for (int i = 0; i < contentLength && !reader.eos() && reader.still_connected(); ++i)
            {
                response.body += reader.read_next();
            }
        }
        catch (const std::exception& ex) 
        {
            std::cerr << "Did not get content length from server." << std::endl;
        }
    }

    close(clientFd);

    return response;
}

/// @brief Makes a GET request.
/// @param request The HTTP request.
/// @return A promise that returns an HttpClientRequest.
/// @throw ClientException
std::future<HttpClientResponse> HttpClient::get_async(const HttpClientRequest& request)
{
    return std::async(&HttpClient::make_request, this, "GET", request);
}

/// @brief Makes a POST request.
/// @param request The HTTP request.
/// @return A promise that returns an HttpClientRequest.
/// @throw ClientException
std::future<HttpClientResponse> HttpClient::post_async(const HttpClientRequest& request)
{
    return std::async(&HttpClient::make_request, this, "POST", request);
}

/// @return A string representing the raw HTTP response.
std::string cas::HttpClientResponse::to_string()
{
    std::ostringstream oss;

    oss << "HTTP/1.1 " << statusCode << " " << statusMessage << std::endl;

    for (auto header : headers)
    {
        oss << header.first << ": " << header.second << std::endl;
    }

    oss << std::endl;
    oss << body;

    return oss.str();
}

/// @return A string representing the raw HTTP request.
std::string cas::HttpClientRequest::to_string()
{
    std::ostringstream oss;

    oss << "[TBD] " << path << " HTTP/1.1" << std::endl;

    for (auto header : headers)
    {
        oss << header.first << ": " << header.second << std::endl;
    }

    oss << std::endl;
    oss << body;

    return oss.str();
}

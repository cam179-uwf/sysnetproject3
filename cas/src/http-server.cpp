/**
 * System & Networks II (COP4635)
 * Christian Marcellino, Ryan Waddington
 * 4/7/2025
 * 
 * This file is for our server http logic.
 */

#include "../libs/http-server.hpp"

#include <iostream>
#include <sstream>

#include "../libs/http-server.hpp"
#include "../libs/string-helpers.hpp"
#include "../libs/exceptions.hpp"
#include "../libs/fd-reader.hpp"

using namespace cas;

HttpServer::HttpServer(const int port, const int bufferSize)
{
    _serverFd = 0;
    _port = port;
    _bufferSize = bufferSize;
    _addrlen = sizeof(_address);

    // =====================
    // | initialize server |
    // =====================

    _serverFd = socket(AF_INET, SOCK_STREAM, 0); // use IPv4 and TCP

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

    // set up our tracked file descriptors
    _fds = {
        { .fd = _serverFd, .events = POLLIN }
    };

    // set the socket level options (SOL_SOCKET) to allow for fast restarts (SO_REUSEADDR),
    // and multiple processes/threads sharing the same port (SO_REUSEPORT)
    int opt = 1; // enable options
    setsockopt(_serverFd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

    _address.sin_family = AF_INET; // use IPv4
    _address.sin_addr.s_addr = INADDR_ANY; // 0.0.0.0 (coming from any address)
    _address.sin_port = htons(_port); // set the port to listen on

    // bind the server socket to the newly created address
    if (bind(_serverFd, (sockaddr*)&_address, _addrlen) < 0)
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
        case EACCES: throw ServerException("Server failed to bind the socket: the specified address is protected and the current user does not have permission to bind to it.");
        case EISCONN: throw ServerException("Server failed to bind the socket: the socket is already connected.");
        case ELOOP: throw ServerException("Server failed to bind the socket: more than {SYMLOOP_MAX} symbolic links were encountered during resolution of the pathname in address.");
        case ENAMETOOLONG: throw ServerException("Server failed to bind the socket: the length of a pathname exceeds {PATH_MAX}, or pathname resolution of a symbolic link produced an intermediate result with a length that exceeds {PATH_MAX}.");
        default: throw ServerException("Server failed to bind the socket."); 
        }
    }

    // start listening for client connections
    // up to a max of SOMAXCONN are allowed
    // in the backlog before accept() is called
    if (listen(_serverFd, SOMAXCONN) < 0)
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
}

HttpServer::~HttpServer()
{
    shutdown();
}

HttpServerContext HttpServer::get_ctx()
{
    HttpServerContext result;

    bool hasContext = false;

    // clientFd will stay at zero until
    // a clientFd is actually read
    while (!hasContext)
    {
        // wait for an event but timeout at 10 seconds
        int numberOfEvents = poll(_fds.data(), _fds.size(), 10000);

        if (numberOfEvents <= 0)
        {
            if (VERBOSE_DEBUG && numberOfEvents == 0)
            {
                std::cout << "Poll timed out after 10 seconds." << std::endl;

                std::cout << "Connected Clients: [" << std::endl;
                for (size_t i = 1; i < _fds.size(); ++i)
                {
                    std::cout << "\tClientFd: " << _fds[i].fd << std::endl;
                }
                std::cout << "]" << std::endl;
            }

            continue;
        }
        
        // process event
        for (size_t i = 0; i < _fds.size(); ++i)
        {
            if ((_fds[i].revents & POLLIN))
            {
                if (_fds[i].fd == _serverFd)
                {
                    handleAccept();
                }
                else
                {
                    hasContext = handleRead(_fds[i].fd, i, result);
                }   
                break;
            }
        }
    }

    if (VERBOSE_DEBUG)
    {
        std::cout << "Returning a context with clientFd: " << result.fd << std::endl;
    }
    return result;
}

bool cas::HttpServer::handleRead(int clientFd, size_t& fdIndex, HttpServerContext& result)
{
    std::ostringstream oss;
    int seqCheck = 0;
    FdReader reader(clientFd, _bufferSize);

    // read the incoming client connection up to a max of _bufferSize in bytes.
    // TODO: if the buffer is not large enough an error will most likely be thrown?
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

    result.request.init_from_raw_http_header(oss.str()); // Parse the client connection assuming HTTP protocol
    
    if (result.request.headers_contain("Content-Length"))
    {
        try
        {
            int contentLength = std::stoi(result.request.headers["Content-Length"]);

            if (VERBOSE_DEBUG)
            {
                std::cout << "Read body of content length: " << contentLength << std::endl;
            }

            for (int i = 0; i < contentLength && !reader.eos() && reader.still_connected(); ++i)
            {
                result.request.body += reader.read_next();
            }
        }
        catch (const std::exception& ex)
        {
            if (VERBOSE_DEBUG)
            {
                std::cerr << "Did not get content length from client." << std::endl;
            }
        }
    }

    if (VERBOSE_DEBUG)
    {
        std::cout << "Read from client: " << clientFd << std::endl;
    }

    if (!reader.still_connected())
    {
        close(clientFd);
        _fds.erase(_fds.begin() + fdIndex);
        --fdIndex; // adjust index due to erase

        OnCloseClientConnection(clientFd);
    }
    else
    {
        result.fd = clientFd;
        result.server = this;
        return true;
    }

    return false;
}

void HttpServer::handleAccept()
{
    int clientFd = accept(_serverFd, (sockaddr *)&_address, (socklen_t *)&_addrlen);
    OnOpenClientConnection(clientFd);

    if (VERBOSE_DEBUG)
    {
        std::cout << "Accepted new client: " << clientFd << std::endl;
    }

    // handle errors
    if (clientFd < 0)
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
        default: throw ServerException("Failed to accept an incoming message. (" + std::to_string(clientFd) + ")");
        }
    }

    _fds.push_back({clientFd, POLLIN, 0});
}

std::future<HttpServerContext> HttpServer::get_ctx_async()
{
    return std::async(std::launch::async, &HttpServer::get_ctx, this);
}

void cas::HttpServer::set_port(const int port)
{
    if (VERBOSE_DEBUG)
    {
        std::cout << "Server port was set to: " << port << std::endl;
    }

    _port = port;
}

void cas::HttpServer::set_buffer_size(const int size)
{
    if (VERBOSE_DEBUG)
    {
        std::cout << "Server buffer size was set to: " << size << std::endl;
    }

    _bufferSize = size;
}

void cas::HttpServer::shutdown()
{
    if (VERBOSE_DEBUG)
    {
        std::cout << "Shutting server down..." << std::endl;
    }

    close(_serverFd);
}

bool cas::HttpServer::is_client_connected(int clientFd)
{
    for (auto fd : _fds)
    {
        if (fd.fd == clientFd)
        {
            return true;
        }
    }

    return false;
}

void cas::HttpServer::close_client_connection(int clientFd)
{
    if (VERBOSE_DEBUG)
    {
        std::cout << "Manually closing clientFd: " << clientFd << std::endl;
    }

    for (size_t i = 0; i < _fds.size(); ++i)
    {
        if (_fds[i].fd == clientFd)
        {
            close(clientFd);
            _fds.erase(_fds.begin() + i);
            
            OnCloseClientConnection(clientFd);
            break;
        }
    }
}

std::future<void> HttpServerContext::send_response_async()
{
    if (_wasSent) throw std::runtime_error("[send_response_async] Policy dictates that their can only be one response sent per context.");
    _wasSent = true;
    
    return std::async([this]() {
        response.headers["Content-Length"] = std::to_string(response.body.size());
        std::string content(response.to_string());

        if (VERBOSE_DEBUG)
        {
            std::cout << "Sending message to clientFd: " << fd << std::endl;
        }

        // send the response to the client
        if (send(fd, content.c_str(), content.size(), 0) < 0)
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

std::future<void> HttpServerContext::send_response_and_close_async()
{
    if (_wasSent) throw std::runtime_error("[send_response_and_close_async] Policy dictates that their can only be one response sent per context.");

    return std::async([this]() {
        send_response_async().get();

        // WARNING: this could fail if the pointer is dereferenced before this is called
        server->close_client_connection(fd);
    });
}
/**
 * Christian Marcellino
 * 4/7/2025
 * 
 * This file is for our server http logic.
 */

#include "../libs/http-server.hpp"

#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sstream>

#include "../libs/http-server.hpp"
#include "../libs/string-helpers.hpp"
#include "../libs/exceptions.hpp"

using namespace cas;

HttpServer::HttpServer(const int port, const int bufferSize)
{
    _serverFd = 0;
    _port = port;
    _bufferSize = bufferSize;
}

HttpServer::~HttpServer()
{
    close(_serverFd);
}

cas::HttpServer::HttpServer(const HttpServer &other)
{
    _serverFd = other._serverFd;
    _port = other._port;
    _bufferSize = other._bufferSize;
}

HttpServer &cas::HttpServer::operator=(const HttpServer &other)
{
    if (this != &other)
    {
        _serverFd = other._serverFd;
        _port = other._port;
        _bufferSize = other._bufferSize;
    }
    return *this;
}

/// @brief Gets an HTTP context for a single transaction. Blocks until a client request is available.
/// @return The HttpServerContext.
/// @throw ServerException
HttpServerContext HttpServer::get_ctx()
{
    HttpServerContext result;

    int clientFd;
    sockaddr_in address;
    int addrlen = sizeof(address);

    // if a server socket has not been created already,
    // create a server socket and start listening for 
    // client connections
    if (_serverFd <= 0)
    {
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

        // set the socket level options (SOL_SOCKET) to allow for fast restarts (SO_REUSEADDR),
        // and multiple processes/threads sharing the same port (SO_REUSEPORT)
        int opt = 1; // enable options
        setsockopt(_serverFd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

        address.sin_family = AF_INET; // use IPv4
        address.sin_addr.s_addr = INADDR_ANY; // 0.0.0.0 (coming from any address)
        address.sin_port = htons(_port); // set the port to listen on

        // bind the server socket to the newly created address
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

    // accept any incoming client connections.
    clientFd = accept(_serverFd, (sockaddr*)&address, (socklen_t*)&addrlen);

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

    std::ostringstream oss;
    char buffer[_bufferSize] = {0};

    // read the incoming client connection up to a max of _bufferSize in bytes.
    // TODO: if the buffer is not large enough an error will most likely be thrown?
    auto readResult = read(clientFd, buffer, _bufferSize - 1);
    buffer[_bufferSize - 1] = '\0';
    oss << buffer;

    if (readResult < 0)
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
    
    // Parse the client connection assuming HTTP protocol
    result.request.parse(oss.str());

    result.response.set_client_fd(clientFd);
    return result;
}

/// @brief Gets an HTTP context for a single transaction. If awaited, blocks until a client request is available.
/// @return A promise that returns the HttpServerContext.
/// @throw ServerException 
std::future<HttpServerContext> HttpServer::get_ctx_async()
{
    return std::async(std::launch::async, &HttpServer::get_ctx, this);
}

/// @brief Set the server's port.
/// @param port The port for the server to use.
void cas::HttpServer::set_port(const int port)
{
    _port = port;
}

/// @brief Set the server's buffer size for reading client requests.
/// @param size The buffer size.
void cas::HttpServer::set_buffer_size(const int size)
{
    _bufferSize = size;
}

/// @brief Closes the server's socket.
void cas::HttpServer::shutdown()
{
    close(_serverFd);
}
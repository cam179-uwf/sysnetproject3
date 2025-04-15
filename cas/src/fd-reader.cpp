/**
 * Christian Marcellino
 * 4/15/2025
 * 
 * For reading file descriptors naturally.
 */

#include "../libs/fd-reader.hpp"

#include <unistd.h>
#include <iostream>
#include "../libs/exceptions.hpp"

using namespace cas;

FdReader::FdReader(int fd, int bufferSize)
{
    _fd = fd;
    _bufferSize = bufferSize;
    _index = 0;
    _nBytes = 0;
    _connected = true;

    _buffer = new char[bufferSize];
    _buffer[_index] = 0;
}

FdReader::~FdReader()
{
    delete[] _buffer;
}

char FdReader::read_next()
{
    if (eos()) // ready for more
    {
        if (VERBOSE_DEBUG)
        {
            std::cout << "Reading from fd: " << _fd << std::endl;
            std::cout << "FdReader: [" << std::endl;
            std::cout << "\tFd: " << _fd << std::endl;
            std::cout << "\tBufferSize: " << _bufferSize << std::endl;
            std::cout << "\tIndex: " << _index << std::endl;
            std::cout << "\tNBytes: " << _nBytes << std::endl;
            std::cout << "\tConnected: " << (_connected ? "True" : "False") << std::endl;
            std::cout << "]" << std::endl;
        }

        _nBytes = read(_fd, _buffer, _bufferSize - 1);
        _buffer[_nBytes] = '\0';
        _index = 0;

        if (_nBytes == 0)
        {
            _connected = false;
            return '\0';
        }
        else if (_nBytes < 0)
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
    }

    char next = _buffer[_index];
    ++_index;
    return next;
}

bool FdReader::eos()
{
    return _buffer[_index] == 0 || _index == _bufferSize - 1;
}

bool cas::FdReader::still_connected()
{
    return _connected;
}

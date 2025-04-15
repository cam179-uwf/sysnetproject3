/**
 * Christian Marcellino, Ryan Waddington
 * 4/15/2025
 * 
 * For reading file descriptors naturally.
 */

#ifndef H_FD_READER
#define H_FD_READER

namespace cas
{
    /// @brief Used for reading from a file descriptor.
    class FdReader
    {
        int _fd;
        int _bufferSize;
        int _index;
        int _nBytes;
        char* _buffer;

        bool _connected;

    public:
        FdReader(int fd, int bufferSize);
        ~FdReader();
        FdReader(FdReader& other) = default;
        FdReader& operator=(FdReader& other) = default;

        /// @brief Read the next character from the file descriptor.
        /// @return The next character from the file descriptor.
        /// @throws ServerException
        char read_next();

        /// @brief Determine if the end of the stream has been reached.
        /// @return True if the end of the stream has been reached, false otherwise.
        bool eos();
        
        /// @brief Determine if the file descriptor is still connected and in use.
        /// @return True if the file descriptor is still connected and in use, false otherwise.
        bool still_connected();
    };
}

#endif
#ifndef H_FD_READER
#define H_FD_READER

namespace cas
{
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

        char read_next();
        bool eos();
        bool still_connected();
    };
}

#endif
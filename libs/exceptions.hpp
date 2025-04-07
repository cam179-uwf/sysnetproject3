#ifndef H_EXCEPTIONS
#define H_EXCEPTIONS

#include <stdexcept>

namespace cas 
{
    class ServerException : public std::runtime_error
    {
    public:
        using std::runtime_error::runtime_error;
    };

    class ClientException : public std::runtime_error
    {
    public:
        using std::runtime_error::runtime_error;
    };
}

#endif
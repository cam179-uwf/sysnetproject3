#ifndef H_SERVER_EXCEPTION
#define H_SERVER_EXCEPTION

#include <stdexcept>

namespace cas 
{
    class ServerException : public std::runtime_error
    {
    public:
        using std::runtime_error::runtime_error;
    };
}

#endif
/**
 * Christian Marcellino
 * 4/7/2025
 * 
 * This file is for any exceptions
 * that our client-server libraries rely on
 */

#ifndef H_EXCEPTIONS
#define H_EXCEPTIONS

#include <stdexcept>

#define VERBOSE_DEBUG true

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
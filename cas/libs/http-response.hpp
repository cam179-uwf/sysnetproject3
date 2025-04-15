/**
 * System & Networks II (COP4635)
 * Christian Marcellino, Ryan Waddington
 * 4/15/2025
 * 
 * For handling HTTP responses.
 */

#ifndef H_HTTP_RESPONSE
#define H_HTTP_RESPONSE

#include <string>
#include <map>

namespace cas 
{
    enum class HttpStatus
    {
        Continue,
        SwitchingProtocols,
        OK,
        Created,
        Accepted,
        NoContent,
        MovedPermanently,
        Found,
        SeeOther,
        NotModified,
        TemporaryRedirect,
        PermanentRedirect,
        BadRequest,
        Unauthorized,
        Forbidden,
        NotFound,
        MethodNotAllowed,
        RequestTimeout,
        Conflict,
        LengthRequired,
        PayloadTooLarge,
        UnsupportedMediaType,
        UnprocessableEntity,
        TooManyRequests,
        InternalServerError,
        NotImplemented,
        BadGateway,
        ServiceUnavailable,
        GatewayTimeout
    };

    struct HttpResponse
    {
        std::string protocol = "HTTP/1.1";
        int statusCode = 200;
        std::string statusMessage = "Success";
        std::map<std::string, std::string> headers;
        std::string body = "";

        /// @brief Initiates response fields using a raw http header.
        void init_from_raw_http_header(const std::string& rawHttpHeader);

        /// @brief Determines if the headers contain the specified key.
        /// @return True if the key exists in the headers, false otherwise.
        bool headers_contain(const std::string& key) const;

        /// @brief Tries to get the header with the specified key.
        /// @param outValue The referenced value to set if the specified key is found. 
        /// @return True if the key is found, otherwise false.
        bool try_get_header(const std::string& key, std::string& outValue);

        /// @brief Set the response status.
        void set_status(const HttpStatus& status);

        /// @return A string representing the raw http response.
        std::string to_string();
    };
}

#endif
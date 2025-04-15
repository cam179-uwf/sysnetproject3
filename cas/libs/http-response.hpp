/**
 * Christian Marcellino, Ryan Waddington
 * 4/7/2025
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

        void init_from_raw_http_header(const std::string& rawHttpHeader);
        void set_status(const HttpStatus& status);
        std::string to_string();
    };
}

#endif
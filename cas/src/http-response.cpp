/**
 * Christian Marcellino, Ryan Waddington
 * 4/7/2025
 * 
 * For handling HTTP responses.
 */

#include "../libs/http-response.hpp"

#include <sstream>

#include "../libs/string-helpers.hpp"

using namespace cas;

void HttpResponse::init_from_raw_http_header(const std::string& rawHttpHeader)
{
    std::istringstream iss(rawHttpHeader);

    std::string firstLine;
    std::getline(iss, firstLine);

    auto firstLineSplits = strhelp::split(firstLine, ' ', 3);

    if (firstLineSplits.size() >= 1)
    {
        protocol = firstLineSplits[0];
    }

    if (firstLineSplits.size() >= 2)
    {
        try 
        {
            statusCode = std::stoi(firstLineSplits[1]);
        }
        catch (const std::exception& ex)
        {
            statusCode = -1;
        }
    }

    if (firstLineSplits.size() >= 3)
    {
        statusMessage = firstLineSplits[2];
    }

    std::string header;
    std::getline(iss, header);

    while (header.size() > 0 && !iswspace(header[0]))
    {
        auto headerSplits = strhelp::split(header, ':');

        if (headerSplits.size() == 2)
        {
            headers[headerSplits[0]] = strhelp::trim(headerSplits[1]);
        }

        std::getline(iss, header);
    }
}

void HttpResponse::set_status(const HttpStatus& status)
{
    switch (status)
    {
    case HttpStatus::Continue:
        statusCode = 100;
        statusMessage = "Continue";
        break;
    case HttpStatus::SwitchingProtocols: 
        statusCode = 101;
        statusMessage = "Switching Protocols";
        break;
    case HttpStatus::OK:
        statusCode = 200;
        statusMessage = "OK";
        break;
    case HttpStatus::Created:
        statusCode = 201;
        statusMessage = "Created";
        break;
    case HttpStatus::Accepted:
        statusCode = 202;
        statusMessage = "Accepted";
        break;
    case HttpStatus::NoContent:
        statusCode = 204;
        statusMessage = "No Content";
        break;
    case HttpStatus::MovedPermanently:
        statusCode = 301;
        statusMessage = "Moved Permanently";
        break;
    case HttpStatus::Found:
        statusCode = 302;
        statusMessage = "Found";
        break;
    case HttpStatus::SeeOther:
        statusCode = 303;
        statusMessage = "See Other";
        break;
    case HttpStatus::NotModified:
        statusCode = 304;
        statusMessage = "Not Modified";
        break;
    case HttpStatus::TemporaryRedirect:
        statusCode = 307;
        statusMessage = "Temporary Redirect";
        break;
    case HttpStatus::PermanentRedirect:
        statusCode = 308;
        statusMessage = "Permanent Redirect";
        break;
    case HttpStatus::BadRequest:
        statusCode = 400;
        statusMessage = "Bad Request";
        break;
    case HttpStatus::Unauthorized:
        statusCode = 401;
        statusMessage = "Unauthorized";
        break;
    case HttpStatus::Forbidden:
        statusCode = 403;
        statusMessage = "Forbidden";
        break;
    case HttpStatus::NotFound:
        statusCode = 404;
        statusMessage = "Not Found";
        break;
    case HttpStatus::MethodNotAllowed:
        statusCode = 405;
        statusMessage = "Method Not Allowed";
        break;
    case HttpStatus::RequestTimeout:
        statusCode = 408;
        statusMessage = "Request Timeout";
        break;
    case HttpStatus::Conflict:
        statusCode = 409;
        statusMessage = "Conflict";
        break;
    case HttpStatus::LengthRequired:
        statusCode = 411;
        statusMessage = "Length Required";
        break;
    case HttpStatus::PayloadTooLarge:
        statusCode = 413;
        statusMessage = "Payload Too Large";
        break;
    case HttpStatus::UnsupportedMediaType:
        statusCode = 415;
        statusMessage = "Unsupported Media Type";
        break;
    case HttpStatus::UnprocessableEntity:
        statusCode = 422;
        statusMessage = "Unprocessable Entity";
        break;
    case HttpStatus::TooManyRequests:
        statusCode = 429;
        statusMessage = "Too Many Requests";
        break;
    case HttpStatus::InternalServerError:
        statusCode = 500;
        statusMessage = "Internal Server Error";
        break;
    case HttpStatus::NotImplemented:
        statusCode = 501;
        statusMessage = "Not Implemented";
        break;
    case HttpStatus::BadGateway:
        statusCode = 502;
        statusMessage = "Bad Gateway";
        break;
    case HttpStatus::ServiceUnavailable:
        statusCode = 503;
        statusMessage = "Service Unavailable";
        break;
    case HttpStatus::GatewayTimeout:
        statusCode = 504;
        statusMessage = "Gateway Timeout";
        break;
    }
}

std::string HttpResponse::to_string()
{
    std::ostringstream oss;

    oss << protocol << " " << statusCode << " " << statusMessage << "\r\n";

    for (auto header : headers)
    {
        oss << header.first << ": " << header.second << "\r\n";
    }

    oss << "\r\n";
    oss << body;

    return oss.str();
}

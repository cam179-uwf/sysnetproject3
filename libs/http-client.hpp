#ifndef H_HTTP_CLIENT
#define H_HTTP_CLIENT

#include <string>
#include <future>
#include <map>

// cas (Client and Server)
namespace cas
{
    struct HttpClientResponse
    {
        std::string httpVersion;
        int statusCode;
        std::string statusMessage;
        std::map<std::string, std::string> headers;
        std::string body;
    };

    struct HttpClientRequest
    {
        std::string path;
        std::map<std::string, std::string> headers;
        std::string body;
    };

    class HttpClient
    {
        std::string _host;
        int _port;

        HttpClientResponse make_request(const std::string& method, const HttpClientRequest& request);

    public:
        HttpClient(const std::string& host, const int port);
        ~HttpClient();
        HttpClient(const HttpClient& other);
        HttpClient& operator=(const HttpClient& other);    

        std::future<HttpClientResponse> get_async(const HttpClientRequest& request);
        std::future<HttpClientResponse> post_async(const HttpClientRequest& request);
    };
}

#endif
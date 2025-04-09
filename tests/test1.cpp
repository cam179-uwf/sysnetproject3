#include <iostream>

#include "TESTS.hpp"

#include "../cas/libs/http-client.hpp"
#include "../cas/libs/http-server.hpp"
#include "../cas/libs/string-helpers.hpp"

TESTS()
{
    // cas::HttpServer server(60001, 1024);
    // auto contextAsync = server.get_ctx_async();

    // cas::HttpClient client("127.0.0.1", 60001, 4096);
    // cas::HttpClientRequest request;
    // request.path = "/";
    // auto clientResponseAsync = client.get_async(request);

    // auto context = contextAsync.get();

    // std::cout << "Client Request: " << std::endl;
    // std::cout << context.request.to_string() << std::endl;

    // context.response.protocol = "HTTP/1.1";
    // context.response.statusCode = 200;
    // context.response.statusMessage = "Success";
    // context.response.body = "This worked!";
    // context.response.sendoff_async().get();

    // auto clientResponse = clientResponseAsync.get();

    // std::cout << "Client Response: " << std::endl;
    // std::cout << clientResponse.to_string() << std::endl;

    auto values = strhelp::split("Hello, world for me!", ' ', 2);

    for (auto value : values)
    {
        std::cout << value << std::endl;
    }
}
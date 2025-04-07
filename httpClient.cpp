#include <iostream>

#include "libs/http-client.hpp"

int main(int argc, char** argv)
{
    cas::HttpClient client("127.0.0.1", 60001);

    cas::HttpClientRequest request;
    request.path = "/";
    request.body = "Trying to get the index page.";

    auto response = client.get_async(request).get();

    std::cout << response.httpVersion << std::endl;
    std::cout << response.statusCode << std::endl;
    std::cout << response.statusMessage << std::endl;
    std::cout << response.body << std::endl;
}
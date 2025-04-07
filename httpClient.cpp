#include <iostream>

#include "cas/libs/http-client.hpp"

int main(int argc, char** argv)
{
    cas::HttpClient client("127.0.0.1", 60001, 4096);

    cas::HttpClientRequest request;
    request.path = "/";
    request.body = "Trying to get the index page.";

    std::cout << "Request: [" << std::endl;
    std::cout << request.to_string() << std::endl;
    std::cout << "]" << std::endl;

    auto response = client.get_async(request).get();

    std::cout << "Response: [" << std::endl;
    std::cout << response.to_string() << std::endl;
    std::cout << "]" << std::endl;
}
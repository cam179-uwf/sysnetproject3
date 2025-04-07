#include <iostream>

#include "libs/http-server.hpp"

int main(int argc, char** argv)
{
    cas::HttpServer server;

    while (true)
    {
        auto context = server.get_ctx_async().get();

        std::cout << "Accepted message from client" << std::endl;
        std::cout << "Method: " << context.request.get_method() << std::endl;
        std::cout << "Path: " << context.request.get_path() << std::endl;
        std::cout << "Protocol: " << context.request.get_protocol() << std::endl;
        std::cout << "Body: |" << context.request.get_body() << "|" << std::endl;

        context.response.protocol = "HTTP/1.1";
        context.response.statusCode = 200;
        context.response.statusMessage = "Success";
        context.response.body = "<!DOCTYPE html><html><head><title>WOW!</title></head><body><p>This is some information for you!</p></body></html>";

        context.response.send_and_close_async().get();
    }
}
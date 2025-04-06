#include <iostream>

#include "libs/http-server.hpp"

int main(int argc, char** argv)
{
    cas::HttpServer server;

    while (true)
    {
        auto context = server.get_ctx_async().get();

        std::cout << "Accepted message from client" << std::endl;
        std::cout << "Method: " << context.get_req().get_method() << std::endl;
        std::cout << "Path: " << context.get_req().get_path() << std::endl;
        std::cout << "Protocol: " << context.get_req().get_protocol() << std::endl;
        std::cout << "Body: |" << context.get_req().get_body() << "|" << std::endl;

        context.get_res().send_and_close();
    }
}
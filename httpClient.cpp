#include <iostream>
#include <string>

#include "cas/libs/http-client.hpp"

void send_client_message(cas::HttpClient& client)
{
    std::cout << "Type message: ";

    std::string message;
    std::getline(std::cin, message);

    cas::HttpClientRequest request;
    request.path = "/";
    request.body = message;
    request.headers["Content-Length"] = std::to_string(request.body.size());
    request.headers["Connection"] = "keep-alive";

    auto response = client.post_async(request).get();

    std::cout << response.to_string() << std::endl;

    if (response.statusCode != 200)
    {
        std::cerr << "Did not successfully send message." << std::endl;
    }
}

int main(int argc, char** argv)
{
    cas::HttpClient client("127.0.0.1", 60001, DEFAULT_CLIENT_BUFFER_SIZE);

    std::string line;
    int option;

    while (true)
    {
        std::cout << "1. send client message\n";
        std::cout << "Pick an Option >> ";
        
        std::getline(std::cin, line);

        if (line == "exit")
        {
            break;
        }
        else
        {
            try 
            {
                option = std::stoi(line);

                switch (option)
                {
                case 1: send_client_message(client);
                default: continue;
                }
            }
            catch (const std::exception& ex)
            {
                std::cerr << "That is not an option." << std::endl;
            }
        }
    }
}
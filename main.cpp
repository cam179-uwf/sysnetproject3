#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <csignal>
#include <cstdlib>

#include "libs/http-server.hpp"

cas::HttpServer g_Server;

std::string read_file_contents(std::string path)
{
    std::cout << "Reading file contents from path: " << path << std::endl;

    std::ifstream ifs(path);
    std::ostringstream oss;
    
    if (ifs.is_open())
    {
        std::string line;
        while (std::getline(ifs, line))
        {
            oss << line;
        }

        return oss.str();
    }
    else
    {
        std::cerr << "Could not open " << path << "." << std::endl;
        throw std::runtime_error("Could not open " + path + ".");
    }

    return "File not found.";
}

int main(int argc, char** argv)
{
    g_Server.set_port(60001);

    std::signal(SIGINT, [](int signal) {
        std::cout << "\nDetected (Ctrl + C): shutting down server.\n";
        g_Server.shutdown();
        std::exit(signal);
    });

    while (true)
    {
        auto context = g_Server.get_ctx_async().get();

        std::cout << "Accepted message from client" << std::endl;
        std::cout << "Method: " << context.request.get_method() << std::endl;
        std::cout << "Path: " << context.request.get_path() << std::endl;
        std::cout << "Protocol: " << context.request.get_protocol() << std::endl;
        std::cout << "Body: |" << context.request.get_body() << "|" << std::endl;

        if (context.request.get_method() == "GET")
        {
            context.response.protocol = "HTTP/1.1";
            context.response.statusCode = 200;
            context.response.statusMessage = "Success";

            try 
            {
                if (context.request.get_path() == "/")
                {
                    context.response.body = read_file_contents("www/index.html");
                }
                else
                {
                    context.response.body = read_file_contents("www" + context.request.get_path());
                }
            }
            catch (const std::exception& ex)
            {
                context.response.statusCode = 404;
                context.response.statusMessage = "File not found!";
            }
        }

        context.response.send_and_close_async().get();
    }
}
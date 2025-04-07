#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <csignal>
#include <cstdlib>

#include "cas/libs/http-server.hpp"

cas::HttpServer g_Server(60001, 1024);

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
    std::signal(SIGINT, [](int signal) {
        std::cout << "\nDetected (Ctrl + C): shutting down server.\n";
        g_Server.shutdown();
        std::exit(signal);
    });

    while (true)
    {
        try 
        {
            std::cout << "Waiting for client..." << std::endl;
            
            auto context = g_Server.get_ctx_async().get();

            std::cout << "Request: [" << std::endl;
            std::cout << context.request.to_string() << std::endl;
            std::cout << "]" << std::endl;

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

            std::cout << "Response: [" << std::endl;
            std::cout << context.response.to_string() << std::endl;
            std::cout << "]" << std::endl;

            context.response.send_and_close_async().get();
        }
        catch (const std::exception& ex)
        {
            std::cerr << ex.what() << std::endl;
        }
    }
}
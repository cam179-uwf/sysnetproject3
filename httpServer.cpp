#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <csignal>
#include <cstdlib>

#include "cas/libs/http-server.hpp"

cas::HttpServer g_Server(60001, DEFAULT_SERVER_BUFFER_SIZE);

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

    throw std::runtime_error("File Not Found.");
}

void handleContext(cas::HttpServerContext& ctx)
{
    // std::cout << std::endl;
    // std::cout << "REQUEST: [" << std::endl;
    // std::cout << ctx.request.to_string() << std::endl;
    // std::cout << "]" << std::endl;

    if (ctx.request.get_method() == "GET")
    {
        if (ctx.request.get_path() == "/")
        {
            ctx.response.body = read_file_contents("www/index.html");
            ctx.response.headers["Content-Length"] = std::to_string(ctx.response.body.size());
            ctx.response.headers["Content-Type"] = "text/html";
            ctx.response.headers["Connection"] = "keep-alive";

            ctx.response.sendoff_async().get();
        }
        else
        {
            try 
            {
                ctx.response.body = read_file_contents("www" + ctx.request.get_path());
                ctx.response.headers["Content-Length"] = std::to_string(ctx.response.body.size());
                ctx.response.headers["Content-Type"] = "text/html";
                ctx.response.headers["Connection"] = "keep-alive";

                ctx.response.sendoff_async().get();
            }
            catch (const std::exception& ex)
            {
                ctx.response.headers["Content-Length"] = std::to_string(ctx.response.body.size());
                ctx.response.headers["Content-Type"] = "text/html";
                ctx.response.headers["Connection"] = "keep-alive";
                ctx.response.statusCode = 404;
                ctx.response.statusMessage = "File Not Found";

                ctx.response.sendoff_async().get();
            }
        }
    }
    else if (ctx.request.get_method() == "POST")
    {
        std::cout << "Message from client: " << ctx.request.get_body() << std::endl;

        ctx.response.headers["Content-Length"] = "0";
        ctx.response.headers["Content-Type"] = "text/html";
        ctx.response.headers["Connection"] = "keep-alive";

        ctx.response.sendoff_async().get();
    }
    else
    {
        ctx.response.headers["Content-Length"] = std::to_string(ctx.response.body.size());
        ctx.response.headers["Content-Type"] = "text/html";
        ctx.response.headers["Connection"] = "close";
        ctx.response.statusCode = 500;
        ctx.response.statusMessage = "Internal Server Error";

        ctx.response.sendoff_async().get();
    }
}

int main(int argc, char** argv)
{
    g_Server.OnCloseClientConnection = [](int clientId) {
        // handle a closing client connection
        std::cout << "Connection was closed: " << clientId << std::endl;
    };

    while (true)
    {
        auto ctx = g_Server.get_ctx_async().get();
        handleContext(ctx);
    }
}
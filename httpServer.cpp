#include <iostream>
#include <fstream>
#include <vector>

#include "cas/libs/http-server.hpp"
#include "libs/helpers.hpp"
#include "cas/libs/string-helpers.hpp"
#include "libs/user-info.hpp"
#include "libs/weather-service.hpp"
#include "cas/libs/exceptions.hpp"

WeatherService g_Service;

void handle_context(cas::HttpServerContext& ctx)
{
    if (VERBOSE_DEBUG)
    {
        std::cout << "=======================================" << std::endl;
        std::cout << ctx.request.to_string() << std::endl;
        std::cout << "=======================================" << std::endl;
    }

    if (ctx.request.method == "POST" && ctx.request.path == "/signup")
    {
        g_Service.sign_up(ctx);
    }
    else if (ctx.request.method == "POST" && ctx.request.path == "/login")
    {
        g_Service.log_in(ctx);
    }
    else if (ctx.request.method == "POST" && ctx.request.path == "/logout")
    {
        g_Service.log_out(ctx);
    }
    else if (ctx.request.method == "GET" && ctx.request.path == "/amiloggedin")
    {
        g_Service.is_logged_in(ctx);
    }
    else if (ctx.request.method == "POST" && ctx.request.path == "/changepassword")
    {
        g_Service.change_password(ctx);
    }
    else if (ctx.request.method == "POST" && ctx.request.path == "/subscribe")
    {
        g_Service.subscribe(ctx);
    }
    else if (ctx.request.method == "POST" && ctx.request.path == "/unsubscribe")
    {
        g_Service.unsubscribe(ctx);
    }
    else if (ctx.request.method == "GET" && ctx.request.path == "/getlocations")
    {
        g_Service.get_locations(ctx);
    }
    else if (ctx.request.method == "POST" && ctx.request.path == "/messages")
    {
        ctx.response.set_status(cas::HttpStatus::OK);
        ctx.send_response_async().get();

        std::cout << "Message from client: " << ctx.request.body << std::endl;
    }
    else if (ctx.request.method == "GET")
    {
        try 
        {
            if (ctx.request.path == "/") ctx.request.path = "/index.html";

            auto contents = read_file_contents("www" + ctx.request.path);

            ctx.response.set_status(cas::HttpStatus::OK);
            ctx.response.body = contents;
        }
        catch (const std::exception& ex)
        {
            ctx.response.set_status(cas::HttpStatus::NotFound);
            ctx.response.body = read_file_contents("www/404.html");
        }

        std::string connectionType;
        if (ctx.request.try_get_header("Connection", connectionType) && connectionType == "keep-alive")
        {
            ctx.send_response_async().get();
        }
        else
        {
            ctx.send_response_and_close_async().get();
        }
    }
    else
    {
        std::cout << "Did nothing with the context" << std::endl;

        ctx.response.set_status(cas::HttpStatus::NotFound);
        ctx.send_response_async().get();
    }
}

int main(int argc, char **argv)
{
    g_Service.get_server().OnOpenClientConnection = [](int clientFd) {
        std::cout << "Client with file descriptor " << clientFd << " connected." << std::endl;
    };

    g_Service.get_server().OnCloseClientConnection = [](int clientFd) {
        std::cout << "Client with file descriptor " << clientFd << " disconnected." << std::endl;
    };

    cas::HttpServerContext ctx;

    std::cout << "Starting listening to port 60001 ..." << std::endl;

    while (true)
    {
        try
        {
            auto ctx = g_Service.get_server().get_ctx_async().get();
            handle_context(ctx);
        }
        catch (const std::exception& ex)
        {
            // if any error occurs log it
            std::cerr << ex.what() << std::endl;
        }
    }
}
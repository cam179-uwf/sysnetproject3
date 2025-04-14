#include <iostream>
#include <fstream>
#include <vector>
#include "cas/libs/http-server.hpp"
#include "libs/helpers.hpp"
#include "cas/libs/string-helpers.hpp"
#include "libs/user-info.hpp"
#include "libs/weather-service.hpp"

WeatherService g_Service;

void handle_context(cas::HttpServerContext& ctx)
{
    if (ctx.request.get_method() == "POST" && ctx.request.get_path() == "/signup")
    {
        g_Service.sign_up(ctx);
    }
    else if (ctx.request.get_method() == "POST" && ctx.request.get_path() == "/login")
    {
        g_Service.log_in(ctx);
    }
    else if (ctx.request.get_method() == "POST" && ctx.request.get_path() == "/logout")
    {
        g_Service.log_out(ctx);
    }
    else if (ctx.request.get_method() == "GET" && ctx.request.get_path() == "/amiloggedin")
    {
        g_Service.is_logged_in(ctx);
    }
    else if (ctx.request.get_method() == "POST" && ctx.request.get_path() == "/changepassword")
    {
        g_Service.change_password(ctx);
    }
    else if (ctx.request.get_method() == "POST" && ctx.request.get_path() == "/subscribe")
    {
        g_Service.subscribe(ctx);
    }
    else if (ctx.request.get_method() == "POST" && ctx.request.get_path() == "/unsubscribe")
    {
        g_Service.unsubscribe(ctx);
    }
    else if (ctx.request.get_method() == "GET" && ctx.request.get_path() == "/getlocations")
    {
        g_Service.get_locations(ctx);
    }
}

int main(int argc, char **argv)
{
    g_Service.get_server().OnCloseClientConnection = [](int clientId)
    {
        // handle a closing client connection
    };

    while (true)
    {
        try 
        {
            auto ctx = g_Service.get_server().get_ctx_async().get();
            handle_context(ctx);
        }
        catch (const std::exception& ex)
        {
            std::cerr << ex.what() << std::endl;
        }
    }
}
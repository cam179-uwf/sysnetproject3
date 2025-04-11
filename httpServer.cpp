#include <iostream>
#include <fstream>
#include <vector>
#include "cas/libs/http-server.hpp"
#include "libs/helpers.hpp"
#include "cas/libs/string-helpers.hpp"

struct UserInfo
{
    std::string username;
    std::string password;
};

cas::HttpServer g_Server(60001, DEFAULT_SERVER_BUFFER_SIZE);
std::map<std::string, UserInfo> _loggedInUsers;
std::vector<UserInfo> g_Users;

void save_file()
{
    using namespace std;
    ofstream oss("users.txt");

    for (auto user : g_Users)
    {
        oss << user.username << "," << user.password << endl;
    }
    oss.close();
}

void load_file()
{
    using namespace std;
    g_Users.clear();
    ifstream iss("users.txt");

    if (iss.is_open())
    {
        string line;
        while (getline(iss, line))
        {
            auto data = strhelp::split(line, ',');
            if (data.size() <= 1)
            {
                continue;
            }
            UserInfo userinfo;
            userinfo.username = data[0];
            userinfo.password = data[1];
            g_Users.push_back(userinfo);
        }
        iss.close();
    }
}

void handleContext(cas::HttpServerContext &ctx)
{
    // ===========
    // | SIGN UP |
    // ===========
    if (ctx.request.get_method() == "POST" && ctx.request.get_path() == "/signup")
    {
        UserInfo userInfo;

        if (!ctx.request.try_get_header("Username", userInfo.username) || is_whitespace(userInfo.username))
        {
            ctx.response.set_status(cas::HttpResponse::Status::BadRequest);
            ctx.response.body = "You must include a username when signing up.";
            ctx.response.sendoff_close_async(g_Server).get();
            return;
        }

        if (!ctx.request.try_get_header("Password", userInfo.password) || is_whitespace(userInfo.password))
        {
            ctx.response.set_status(cas::HttpResponse::Status::BadRequest);
            ctx.response.body = "You must include a password when signing up.";
            ctx.response.sendoff_close_async(g_Server).get();
            return;
        }

        for (auto user : g_Users)
        {
            if (user.username == userInfo.username)
            {
                ctx.response.set_status(cas::HttpResponse::Status::BadRequest);
                ctx.response.body = "Username already exists";
                ctx.response.sendoff_close_async(g_Server).get();
                return;
            }
        }
        auto token = generate_token(base64_encode(userInfo.username.substr(0, 5)));
        _loggedInUsers[token] = userInfo;
        g_Users.push_back(userInfo);
        save_file();
        ctx.response.headers["Authorization"] = "Bearer " + token;
        ctx.response.sendoff_close_async(g_Server).get();

        std::cout << "Signed up a user." << std::endl;
        std::cout << "Username: " << userInfo.username << std::endl;
        std::cout << "Password: " << userInfo.password << std::endl;
        std::cout << "Bearer Token: " << token << std::endl;
        return;
    }
    // ==========
    // | LOG IN |
    // ==========
    else if (ctx.request.get_method() == "POST" && ctx.request.get_path() == "/login")
    {
        UserInfo userInfo;

        if (!ctx.request.try_get_header("Username", userInfo.username) || is_whitespace(userInfo.username))
        {
            ctx.response.set_status(cas::HttpResponse::Status::BadRequest);
            ctx.response.body = "You must include a username when login.";
            ctx.response.sendoff_close_async(g_Server).get();
            return;
        }

        if (!ctx.request.try_get_header("Password", userInfo.password) || is_whitespace(userInfo.password))
        {
            ctx.response.set_status(cas::HttpResponse::Status::BadRequest);
            ctx.response.body = "You must include a password when login.";
            ctx.response.sendoff_close_async(g_Server).get();
            return;
        }

        for (auto user : g_Users)
        {
            if (user.username == userInfo.username && user.password == userInfo.password)
            {
                auto token = generate_token(base64_encode(userInfo.username.substr(0, 5)));
                _loggedInUsers[token] = userInfo;

                ctx.response.headers["Authorization"] = "Bearer " + token;
                ctx.response.sendoff_close_async(g_Server).get();

                std::cout << "Logged in a user." << std::endl;
                std::cout << "Username: " << userInfo.username << std::endl;
                std::cout << "Password: " << userInfo.password << std::endl;
                std::cout << "Bearer Token: " << token << std::endl;
                return;
            }
        }

        ctx.response.set_status(cas::HttpResponse::Status::BadRequest);
        ctx.response.body = "Credentials are INVALID";
        ctx.response.sendoff_close_async(g_Server).get();
        return;
    }
    // ===========
    // | LOG OUT |
    // ===========
    else if (ctx.request.get_method() == "POST" && ctx.request.get_path() == "/logout")
    {
        std::string bearer;

        if (!ctx.request.try_get_header("Authorization", bearer) || is_whitespace(ctx.request.get_headers()["Authorization"]))
        {
            ctx.response.set_status(cas::HttpResponse::Status::BadRequest);
            ctx.response.body = "You must include a bearer when logging out.";
            ctx.response.sendoff_close_async(g_Server).get();
            return;
        }

        auto result = strhelp::split(ctx.request.get_headers()["Authorization"], ' ', 2);

        if (result.size() > 1)
        {
            bearer = result[1];
        }
        else
        {
            ctx.response.set_status(cas::HttpResponse::Status::BadRequest);
            ctx.response.body = "You must include a bearer when logging out.";
            ctx.response.sendoff_close_async(g_Server).get();
            return;
        }

        if (_loggedInUsers.find(bearer) != _loggedInUsers.end())
        {
            auto user = _loggedInUsers[bearer];
            std::cout << "Logged out a user." << std::endl;
            std::cout << "Username: " << user.username << std::endl;
            std::cout << "Password: " << user.password << std::endl;
            std::cout << "Bearer Token: " << bearer << std::endl;
            _loggedInUsers.erase(bearer);
        }

        ctx.response.body = "Logged you out.";

        ctx.response.sendoff_async().get();
        g_Server.close_client_connection(ctx.get_client_fd()); // close the connection
        return;
    }
    else
    {
        std::string bearer;
        bool isAuthorized = false;

        if (ctx.request.get_headers().find("Authorization") != ctx.request.get_headers().end())
        {
            auto result = strhelp::split(ctx.request.get_headers()["Authorization"], ' ', 2);

            if (result.size() > 1)
            {
                bearer = result[1];
                std::cout << "Bearer: " << bearer << std::endl;

                if (_loggedInUsers.find(bearer) != _loggedInUsers.end())
                {
                    isAuthorized = true;
                }
                else
                {
                    ctx.response.set_status(cas::HttpResponse::Status::Unauthorized);
                    ctx.response.sendoff_close_async(g_Server).get();
                    return;
                }
            }
            else
            {
                ctx.response.set_status(cas::HttpResponse::Status::Unauthorized);
                ctx.response.sendoff_close_async(g_Server).get();
                return;
            }
        }

        if (isAuthorized)
        {
            ctx.response.body = "You are logged in.";
            ctx.response.sendoff_close_async(g_Server).get();
            return;
        }

        ctx.response.sendoff_close_async(g_Server).get();
    }
}

int main(int argc, char **argv)
{
    load_file();

    g_Server.OnCloseClientConnection = [](int clientId)
    {
        // handle a closing client connection
        std::cout << "Connection was closed: " << clientId << std::endl;
    };

    while (true)
    {
        auto ctx = g_Server.get_ctx_async().get();
        handleContext(ctx);
    }
}

// TODO: make sendoff_async return an error when called a second time
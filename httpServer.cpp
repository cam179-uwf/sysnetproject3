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
std::map<std::string, UserInfo> g_Sessions;
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
    std::string bearer;
    std::string auth;
    bool isAuthorized = false;

    if (ctx.request.try_get_header("Authorization", auth))
    {
        auto data = strhelp::split(auth, ' ', 2);

        if (data.size() > 1)
        {
            bearer = data[1];
            std::cout << "Client sent their bearer token: " << bearer << std::endl;

            if (g_Sessions.find(bearer) != g_Sessions.end())
            {
                isAuthorized = true;
            }   
        }
    }

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

        g_Sessions[token] = userInfo;
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
                g_Sessions[token] = userInfo;

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
        if (!isAuthorized)
        {
            ctx.response.set_status(cas::HttpResponse::Status::Unauthorized);
            ctx.response.body = "You must be logged in to log out you idiot.";
            ctx.response.sendoff_close_async(g_Server).get();
            return;
        }

        std::string bearer;

        if (!ctx.request.try_get_header("Authorization", bearer) || is_whitespace(bearer))
        {
            ctx.response.set_status(cas::HttpResponse::Status::BadRequest);
            ctx.response.body = "You must include a bearer when logging out.";
            ctx.response.sendoff_close_async(g_Server).get();
            return;
        }

        auto data = strhelp::split(bearer, ' ', 2);

        if (data.size() > 1)
        {
            bearer = data[1];
        }
        else
        {
            ctx.response.set_status(cas::HttpResponse::Status::BadRequest);
            ctx.response.body = "You must include a bearer when logging out.";
            ctx.response.sendoff_close_async(g_Server).get();
            return;
        }

        if (g_Sessions.find(bearer) != g_Sessions.end())
        {
            auto user = g_Sessions[bearer];
            std::cout << "Logged out a user." << std::endl;
            std::cout << "Username: " << user.username << std::endl;
            std::cout << "Password: " << user.password << std::endl;
            std::cout << "Bearer Token: " << bearer << std::endl;
            g_Sessions.erase(bearer);
        }

        ctx.response.body = "Logged you out.";

        ctx.response.sendoff_async().get();
        g_Server.close_client_connection(ctx.get_client_fd()); // close the connection
        return;
    }
    // ==================
    // | Am I Logged In |
    // ==================
    else if (ctx.request.get_method() == "GET" && ctx.request.get_path() == "/amiloggedin")
    {
        if (isAuthorized)
        {
            ctx.response.set_status(cas::HttpResponse::Status::OK);
            ctx.response.sendoff_close_async(g_Server).get();
        }
        else
        {
            ctx.response.set_status(cas::HttpResponse::Status::Unauthorized);
            ctx.response.sendoff_close_async(g_Server).get();
        }
    }
    // ===================
    // | Change Password |
    // ===================
    else if (ctx.request.get_method() == "POST" && ctx.request.get_path() == "/changepassword")
    {
        if (!isAuthorized)
        {
            ctx.response.set_status(cas::HttpResponse::Status::Unauthorized);
            ctx.response.sendoff_close_async(g_Server).get();
            return;
        }
        
        std::string oldPassword;
        std::string newPassword;

        if (!ctx.request.try_get_header("oldpassword", oldPassword) || is_whitespace(oldPassword))
        {
            ctx.response.set_status(cas::HttpResponse::Status::BadRequest);
            ctx.response.body = "You must include the oldpassword header when changing your password.";
            ctx.response.sendoff_close_async(g_Server).get();
            return;
        }

        if (!ctx.request.try_get_header("newpassword", newPassword) || is_whitespace(newPassword))
        {
            ctx.response.set_status(cas::HttpResponse::Status::BadRequest);
            ctx.response.body = "You must include the newpassword header when changing your password.";
            ctx.response.sendoff_close_async(g_Server).get();
            return;
        }

        if (g_Sessions[bearer].password == oldPassword)
        {
            g_Sessions[bearer].password = newPassword;

            for (size_t i = 0; i < g_Users.size(); ++i)
            {
                if (g_Users[i].username == g_Sessions[bearer].username)
                {
                    g_Users[i].password = newPassword;
                }
            }

            ctx.response.set_status(cas::HttpResponse::Status::OK);
            ctx.response.body = "Password changed successfully.";
            ctx.response.sendoff_close_async(g_Server).get();

            save_file();
            return;
        }
        
        ctx.response.set_status(cas::HttpResponse::Status::BadRequest);
        ctx.response.body = "Old password did not match.";
        ctx.response.sendoff_close_async(g_Server).get();
        return;
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
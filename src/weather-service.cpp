#include "../libs/weather-service.hpp"

#include <iostream>
#include "../libs/helpers.hpp"
#include "../cas/libs/string-helpers.hpp"

WeatherService::WeatherService()
{
    load_file(_users);
}

cas::HttpServer &WeatherService::get_server()
{
    return _server;
}

void WeatherService::authenticate(cas::HttpServerContext &ctx)
{
    std::string bearer;
    std::string auth;
    _isAuthenticated = false;

    if (ctx.request.try_get_header("Authorization", auth))
    {
        auto data = strhelp::split(auth, ' ', 2);

        if (data.size() > 1)
        {
            bearer = data[1];
            std::cout << "Client sent their bearer token: " << bearer << std::endl;

            if (_sessions.find(bearer) != _sessions.end())
            {
                _isAuthenticated = true;
                _bearer = bearer;
            }   
        }
    }
}

void WeatherService::sign_up(cas::HttpServerContext &ctx)
{
    UserInfo userInfo;

    if (!ctx.request.try_get_header("Username", userInfo.username) || is_whitespace(userInfo.username))
    {
        ctx.response.set_status(cas::HttpResponse::Status::BadRequest);
        ctx.response.body = "You must include a username when signing up.";
        ctx.response.sendoff_close_async(_server).get();
        return;
    }

    if (!ctx.request.try_get_header("Password", userInfo.password) || is_whitespace(userInfo.password))
    {
        ctx.response.set_status(cas::HttpResponse::Status::BadRequest);
        ctx.response.body = "You must include a password when signing up.";
        ctx.response.sendoff_close_async(_server).get();
        return;
    }

    for (auto user : _users)
    {
        if (user.username == userInfo.username)
        {
            ctx.response.set_status(cas::HttpResponse::Status::BadRequest);
            ctx.response.body = "Username already exists";
            ctx.response.sendoff_close_async(_server).get();
            return;
        }
    }

    auto token = generate_token(base64_encode(userInfo.username.substr(0, 5)));

    _sessions[token] = userInfo;
    _users.push_back(userInfo);
    save_file(_users);
    
    ctx.response.headers["Authorization"] = "Bearer " + token;
    ctx.response.sendoff_close_async(_server).get();

    std::cout << "Signed up a user." << std::endl;
    std::cout << "Username: " << userInfo.username << std::endl;
    std::cout << "Password: " << userInfo.password << std::endl;
    std::cout << "Bearer Token: " << token << std::endl;
    return;
}

void WeatherService::log_in(cas::HttpServerContext &ctx)
{
    UserInfo userInfo;

    if (!ctx.request.try_get_header("Username", userInfo.username) || is_whitespace(userInfo.username))
    {
        ctx.response.set_status(cas::HttpResponse::Status::BadRequest);
        ctx.response.body = "You must include a username when login.";
        ctx.response.sendoff_close_async(_server).get();
        return;
    }

    if (!ctx.request.try_get_header("Password", userInfo.password) || is_whitespace(userInfo.password))
    {
        ctx.response.set_status(cas::HttpResponse::Status::BadRequest);
        ctx.response.body = "You must include a password when login.";
        ctx.response.sendoff_close_async(_server).get();
        return;
    }

    for (auto user : _users)
    {
        if (user.username == userInfo.username && user.password == userInfo.password)
        {
            auto token = generate_token(base64_encode(userInfo.username.substr(0, 5)));
            _sessions[token] = userInfo;

            ctx.response.headers["Authorization"] = "Bearer " + token;
            ctx.response.sendoff_close_async(_server).get();

            std::cout << "Logged in a user." << std::endl;
            std::cout << "Username: " << userInfo.username << std::endl;
            std::cout << "Password: " << userInfo.password << std::endl;
            std::cout << "Bearer Token: " << token << std::endl;
            return;
        }
    }

    ctx.response.set_status(cas::HttpResponse::Status::BadRequest);
    ctx.response.body = "Credentials are INVALID";
    ctx.response.sendoff_close_async(_server).get();
}

void WeatherService::log_out(cas::HttpServerContext &ctx)
{
    authenticate(ctx);

    if (!_isAuthenticated)
    {
        ctx.response.set_status(cas::HttpResponse::Status::Unauthorized);
        ctx.response.body = "You must be logged in to log out you idiot.";
        ctx.response.sendoff_close_async(_server).get();
        return;
    }

    std::string bearer;

    if (!ctx.request.try_get_header("Authorization", bearer) || is_whitespace(bearer))
    {
        ctx.response.set_status(cas::HttpResponse::Status::BadRequest);
        ctx.response.body = "You must include a bearer when logging out.";
        ctx.response.sendoff_close_async(_server).get();
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
        ctx.response.sendoff_close_async(_server).get();
        return;
    }

    if (_sessions.find(bearer) != _sessions.end())
    {
        auto user = _sessions[bearer];
        std::cout << "Logged out a user." << std::endl;
        std::cout << "Username: " << user.username << std::endl;
        std::cout << "Password: " << user.password << std::endl;
        std::cout << "Bearer Token: " << bearer << std::endl;
        _sessions.erase(bearer);
    }

    ctx.response.body = "Logged you out.";

    ctx.response.sendoff_async().get();
    _server.close_client_connection(ctx.get_client_fd()); // close the connection
    return;
}

void WeatherService::is_logged_in(cas::HttpServerContext &ctx)
{   
    authenticate(ctx);

    if (_isAuthenticated)
    {
        ctx.response.set_status(cas::HttpResponse::Status::OK);
        ctx.response.sendoff_close_async(_server).get();
    }
    else
    {
        ctx.response.set_status(cas::HttpResponse::Status::Unauthorized);
        ctx.response.sendoff_close_async(_server).get();
    }
}

void WeatherService::change_password(cas::HttpServerContext &ctx)
{
    authenticate(ctx);

    if (!_isAuthenticated)
    {
        ctx.response.set_status(cas::HttpResponse::Status::Unauthorized);
        ctx.response.sendoff_close_async(_server).get();
        return;
    }
    
    std::string oldPassword;
    std::string newPassword;

    if (!ctx.request.try_get_header("oldpassword", oldPassword) || is_whitespace(oldPassword))
    {
        ctx.response.set_status(cas::HttpResponse::Status::BadRequest);
        ctx.response.body = "You must include the oldpassword header when changing your password.";
        ctx.response.sendoff_close_async(_server).get();
        return;
    }

    if (!ctx.request.try_get_header("newpassword", newPassword) || is_whitespace(newPassword))
    {
        ctx.response.set_status(cas::HttpResponse::Status::BadRequest);
        ctx.response.body = "You must include the newpassword header when changing your password.";
        ctx.response.sendoff_close_async(_server).get();
        return;
    }

    if (_sessions[_bearer].password == oldPassword)
    {
        _sessions[_bearer].password = newPassword;

        for (size_t i = 0; i < _users.size(); ++i)
        {
            if (_users[i].username == _sessions[_bearer].username)
            {
                _users[i].password = newPassword;
            }
        }

        ctx.response.set_status(cas::HttpResponse::Status::OK);
        ctx.response.body = "Password changed successfully.";
        ctx.response.sendoff_close_async(_server).get();

        save_file(_users);
        return;
    }
    
    ctx.response.set_status(cas::HttpResponse::Status::BadRequest);
    ctx.response.body = "Old password did not match.";
    ctx.response.sendoff_close_async(_server).get();
}

void WeatherService::subscribe(cas::HttpServerContext &ctx)
{
    authenticate(ctx);

    if (!_isAuthenticated)
    {
        ctx.response.set_status(cas::HttpResponse::Status::Unauthorized);
        ctx.response.sendoff_close_async(_server).get();
        return;
    }

    std::string location;

    if (!ctx.request.try_get_header("location", location) || is_whitespace(location))
    {
        ctx.response.set_status(cas::HttpResponse::Status::BadRequest);
        ctx.response.body = "You must include the location header when subscribing to a location.";
        ctx.response.sendoff_close_async(_server).get();
        return;
    }

    _sessions[_bearer].locations.push_back(location);

    ctx.response.set_status(cas::HttpResponse::Status::OK);
    ctx.response.sendoff_close_async(_server).get();
    return;
}

void WeatherService::unsubscribe(cas::HttpServerContext &ctx)
{
    authenticate(ctx);

    if (!_isAuthenticated)
    {
        ctx.response.set_status(cas::HttpResponse::Status::Unauthorized);
        ctx.response.sendoff_close_async(_server).get();
        return;
    }

    std::string location;

    if (!ctx.request.try_get_header("location", location) || is_whitespace(location))
    {
        ctx.response.set_status(cas::HttpResponse::Status::BadRequest);
        ctx.response.body = "You must include the location header when unsubscribing to a location.";
        ctx.response.sendoff_close_async(_server).get();
        return;
    }

    for (size_t i = 0; i < _sessions[_bearer].locations.size(); ++i)
    {
        if (_sessions[_bearer].locations[i] == location)
        {
            _sessions[_bearer].locations.erase(_sessions[_bearer].locations.begin() + i);

            ctx.response.set_status(cas::HttpResponse::Status::OK);
            ctx.response.sendoff_close_async(_server).get();
            return;
        }
    }

    ctx.response.set_status(cas::HttpResponse::Status::BadRequest);
    ctx.response.body = "The location you tried to remove doesn't exist.";
    ctx.response.sendoff_close_async(_server).get();
    return;
}

void WeatherService::get_locations(cas::HttpServerContext &ctx)
{
    authenticate(ctx);

    if (!_isAuthenticated)
    {
        ctx.response.set_status(cas::HttpResponse::Status::Unauthorized);
        ctx.response.sendoff_close_async(_server).get();
        return;
    }

    ctx.response.set_status(cas::HttpResponse::Status::OK);
    ctx.response.body = "The locations you are subscribed to:\n" + _sessions[_bearer].locations_to_string();
    ctx.response.sendoff_close_async(_server).get();
    return;
}

/**
 * System & Networks II (COP4635)
 * Christian Marcellino, Ryan Waddington
 * 4/15/2025
 * 
 * Service used for making API calls called by the client.
 */

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

            std::cout << std::endl;
            std::cout << "Client tried authorizing with bearer: " << bearer << std::endl;

            if (_sessions.find(bearer) != _sessions.end())
            {
                std::cout << "Client is authorized." << std::endl;
                _isAuthenticated = true;
                _bearer = bearer;
                _sessions[bearer].fd = ctx.fd;
            }   

            std::cout << std::endl;
        }
    }
}

void WeatherService::sign_up(cas::HttpServerContext &ctx)
{
    UserInfo userInfo;

    if (!ctx.request.try_get_header("Username", userInfo.username) || is_whitespace(userInfo.username))
    {
        ctx.response.set_status(cas::HttpStatus::BadRequest);
        ctx.response.body = "You must specify a username.";
        ctx.send_response_async().get();
        return;
    }

    if (!ctx.request.try_get_header("Password", userInfo.password) || is_whitespace(userInfo.password))
    {
        ctx.response.set_status(cas::HttpStatus::BadRequest);
        ctx.response.body = "You must specify a password.";
        ctx.send_response_async().get();
        return;
    }

    for (auto user : _users)
    {
        if (user.username == userInfo.username)
        {
            ctx.response.set_status(cas::HttpStatus::BadRequest);
            ctx.response.body = "That username already exists.";
            ctx.send_response_async().get();
            return;
        }
    }

    auto token = generate_token(base64_encode(userInfo.username.substr(0, 5)));

    _sessions[token] = userInfo;
    _users.push_back(userInfo);
    save_file(_users);
    
    ctx.response.headers["Authorization"] = "Bearer " + token;
    ctx.send_response_async().get();

    std::cout << std::endl;
    std::cout << "Registered a user." << std::endl;
    std::cout << "Username: " << userInfo.username << std::endl;
    std::cout << "Password: " << userInfo.password << std::endl;
    std::cout << std::endl;
    return;
}

void WeatherService::log_in(cas::HttpServerContext &ctx)
{
    UserInfo userInfo;

    if (!ctx.request.try_get_header("Username", userInfo.username) || is_whitespace(userInfo.username))
    {
        ctx.response.set_status(cas::HttpStatus::BadRequest);
        ctx.response.body = "You must specify a username.";
        ctx.send_response_async().get();
        return;
    }

    if (!ctx.request.try_get_header("Password", userInfo.password) || is_whitespace(userInfo.password))
    {
        ctx.response.set_status(cas::HttpStatus::BadRequest);
        ctx.response.body = "You must specify a password.";
        ctx.send_response_async().get();
        return;
    }

    for (auto user : _users)
    {
        if (user.username == userInfo.username && user.password == userInfo.password)
        {
            auto token = generate_token(base64_encode(userInfo.username.substr(0, 5)));
            _sessions[token] = userInfo;

            ctx.response.headers["Authorization"] = "Bearer " + token;
            ctx.send_response_async().get();

            std::cout << std::endl;
            std::cout << "Logged in user." << std::endl;
            std::cout << "Username: " << userInfo.username << std::endl;
            std::cout << "Password: " << userInfo.password << std::endl;
            std::cout << std::endl;
            return;
        }
    }

    ctx.response.set_status(cas::HttpStatus::BadRequest);
    ctx.response.body = "Your credentials are invalid.";
    ctx.send_response_async().get();
}

void WeatherService::log_out(cas::HttpServerContext &ctx)
{
    authenticate(ctx);

    if (!_isAuthenticated)
    {
        ctx.response.set_status(cas::HttpStatus::Unauthorized);
        ctx.response.body = "You are not logged in.";
        ctx.send_response_async().get();
        return;
    }

    std::string bearer;

    if (!ctx.request.try_get_header("Authorization", bearer) || is_whitespace(bearer))
    {
        ctx.response.set_status(cas::HttpStatus::BadRequest);
        ctx.response.body = "You must specify a bearer token.";
        ctx.send_response_async().get();
        return;
    }

    auto data = strhelp::split(bearer, ' ', 2);

    if (data.size() > 1)
    {
        bearer = data[1];
    }
    else
    {
        ctx.response.set_status(cas::HttpStatus::BadRequest);
        ctx.response.body = "You must specify a properly formatted bearer token.";
        ctx.send_response_async().get();
        return;
    }

    if (_sessions.find(bearer) != _sessions.end())
    {
        auto user = _sessions[bearer];
        _sessions.erase(bearer);

        std::cout << std::endl;
        std::cout << "Logged out a user." << std::endl;
        std::cout << "Username: " << user.username << std::endl;
        std::cout << "Password: " << user.password << std::endl;
        std::cout << "Bearer: " << bearer << std::endl;
        std::cout << std::endl;

        ctx.response.body = "Logged you out.";
        ctx.send_response_async().get();
        return;
    }

    ctx.response.set_status(cas::HttpStatus::NotFound);
    ctx.response.body += ": could not find the logged in user.";
    ctx.send_response_async().get();
}

void WeatherService::is_logged_in(cas::HttpServerContext &ctx)
{   
    authenticate(ctx);

    if (_isAuthenticated)
    {
        ctx.response.set_status(cas::HttpStatus::OK);
        ctx.send_response_async().get();
    }
    else
    {
        ctx.response.set_status(cas::HttpStatus::Unauthorized);
        ctx.send_response_async().get();
    }
}

void WeatherService::change_password(cas::HttpServerContext &ctx)
{
    authenticate(ctx);

    if (!_isAuthenticated)
    {
        ctx.response.set_status(cas::HttpStatus::Unauthorized);
        ctx.send_response_async().get();
        return;
    }
    
    std::string oldPassword;
    std::string newPassword;

    if (!ctx.request.try_get_header("oldpassword", oldPassword) || is_whitespace(oldPassword))
    {
        ctx.response.set_status(cas::HttpStatus::BadRequest);
        ctx.response.body = "You must specify your old password.";
        ctx.send_response_async().get();
        return;
    }

    if (!ctx.request.try_get_header("newpassword", newPassword) || is_whitespace(newPassword))
    {
        ctx.response.set_status(cas::HttpStatus::BadRequest);
        ctx.response.body = "You must specify your new password.";
        ctx.send_response_async().get();
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

        ctx.response.set_status(cas::HttpStatus::OK);
        ctx.response.body = "Your password was changed successfully.";
        ctx.send_response_async().get();

        save_file(_users);
        return;
    }
    
    ctx.response.set_status(cas::HttpStatus::BadRequest);
    ctx.response.body = "The old password specified did not match your original password.";
    ctx.send_response_async().get();
}

void WeatherService::subscribe(cas::HttpServerContext &ctx)
{
    authenticate(ctx);

    if (!_isAuthenticated)
    {
        ctx.response.set_status(cas::HttpStatus::Unauthorized);
        ctx.send_response_async().get();
        return;
    }

    std::string location;

    if (!ctx.request.try_get_header("location", location) || is_whitespace(location))
    {
        ctx.response.set_status(cas::HttpStatus::BadRequest);
        ctx.response.body = "You must specify a location.";
        ctx.send_response_async().get();
        return;
    }

    _sessions[_bearer].locations.push_back(location);

    ctx.response.set_status(cas::HttpStatus::OK);
    ctx.response.body = "Successfully subscribed to: " + location;
    ctx.send_response_async().get();
    return;
}

void WeatherService::unsubscribe(cas::HttpServerContext &ctx)
{
    authenticate(ctx);

    if (!_isAuthenticated)
    {
        ctx.response.set_status(cas::HttpStatus::Unauthorized);
        ctx.send_response_async().get();
        return;
    }

    std::string location;

    if (!ctx.request.try_get_header("location", location) || is_whitespace(location))
    {
        ctx.response.set_status(cas::HttpStatus::BadRequest);
        ctx.response.body = "You must specify a location.";
        ctx.send_response_async().get();
        return;
    }

    for (size_t i = 0; i < _sessions[_bearer].locations.size(); ++i)
    {
        if (_sessions[_bearer].locations[i] == location)
        {
            _sessions[_bearer].locations.erase(_sessions[_bearer].locations.begin() + i);

            ctx.response.set_status(cas::HttpStatus::OK);
            ctx.response.body = "Successfully unsubscribed from: " + location;
            ctx.send_response_async().get();
            return;
        }
    }

    ctx.response.set_status(cas::HttpStatus::BadRequest);
    ctx.response.body = "No location removed. It wasn't in your subscribed list of locations.";
    ctx.send_response_async().get();
    return;
}

void WeatherService::get_locations(cas::HttpServerContext &ctx)
{
    authenticate(ctx);

    if (!_isAuthenticated)
    {
        ctx.response.set_status(cas::HttpStatus::Unauthorized);
        ctx.send_response_async().get();
        return;
    }

    ctx.response.set_status(cas::HttpStatus::OK);
    ctx.response.body = _sessions[_bearer].locations_to_string();
    ctx.send_response_async().get();
    return;
}

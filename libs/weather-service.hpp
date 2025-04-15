/**
 * System & Networks II (COP4635)
 * Christian Marcellino, Ryan Waddington
 * 4/15/2025
 * 
 * Service used for making API calls called by the client.
 */

#ifndef H_WEATHER_SERVICE
#define H_WEATHER_SERVICE

#include "../cas/libs/http-server.hpp"
#include "user-info.hpp"

class WeatherService
{
    cas::HttpServer _server = cas::HttpServer(60001, DEFAULT_SERVER_BUFFER_SIZE);
    std::map<std::string, UserInfo> _sessions;
    std::vector<UserInfo> _users;
    bool _isAuthenticated = false;
    std::string _bearer;

    /// @brief Ensures that a user is authorized.
    void authenticate(cas::HttpServerContext &ctx);

public:
    WeatherService();
    ~WeatherService() = default;
    WeatherService(WeatherService& other) = default;
    WeatherService& operator=(WeatherService& other) = default;

    /// @brief Gets the server that this service is operating with. 
    cas::HttpServer& get_server();

    /// @brief Registers a user.
    void sign_up(cas::HttpServerContext &ctx);

    /// @brief Logs a user in.
    void log_in(cas::HttpServerContext &ctx);

    /// @brief Logs a user out.
    void log_out(cas::HttpServerContext &ctx);

    /// @brief Checks to see if a user is logged in.
    void is_logged_in(cas::HttpServerContext &ctx);

    /// @brief Change the password of a user.
    void change_password(cas::HttpServerContext &ctx);

    /// @brief Subscribe a user to a location. 
    void subscribe(cas::HttpServerContext &ctx);

    /// @brief Unsubscribe a user from a location. 
    void unsubscribe(cas::HttpServerContext &ctx);

    /// @brief Get all the locations that a user has and echo those locations back to the user.
    void get_locations(cas::HttpServerContext &ctx);
};

#endif
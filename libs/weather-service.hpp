#ifndef H_WEATHER_SERVICE
#define H_WEATHER_SERVICE

#include "../cas/libs/http-server-context.hpp"
#include "../cas/libs/http-server.hpp"
#include "user-info.hpp"

class WeatherService
{
    cas::HttpServer _server = cas::HttpServer(60001, DEFAULT_SERVER_BUFFER_SIZE);
    std::map<std::string, UserInfo> _sessions;
    std::vector<UserInfo> _users;
    bool _isAuthenticated = false;
    std::string _bearer;

    void authenticate(cas::HttpServerContext &ctx);

public:
    WeatherService();
    ~WeatherService() = default;
    WeatherService(WeatherService& other) = default;
    WeatherService& operator=(WeatherService& other) = default;

    cas::HttpServer& get_server();
    void sign_up(cas::HttpServerContext &ctx);
    void log_in(cas::HttpServerContext &ctx);
    void log_out(cas::HttpServerContext &ctx);
    void is_logged_in(cas::HttpServerContext &ctx);
    void change_password(cas::HttpServerContext &ctx);
    void subscribe(cas::HttpServerContext &ctx);
    void unsubscribe(cas::HttpServerContext &ctx);
    void get_locations(cas::HttpServerContext &ctx);
};

#endif
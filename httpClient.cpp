#include <iostream>
#include <string>

#include "cas/libs/http-client.hpp"

std::string bearer;

void signup(cas::HttpClient& client)
{
    std::cout << "Type username: ";

    std::string username;
    std::getline(std::cin, username);

    std::cout << "Type password: ";

    std::string password;
    std::getline(std::cin, password);

    cas::HttpClientRequest request;
    request.path = "/signup";
    request.headers["Username"] = username;
    request.headers["Password"] = password;

    auto response = client.post_async(request).get();

    std::cout << response.to_string() << std::endl;

    if (response.statusCode != 200)
    {
        std::cerr << "Did not successfully send message." << std::endl;
    }
    else
    {
        bearer = response.headers["Authorization"];
    }
}

void login(cas::HttpClient& client)
{
    std::cout << "Type username: ";

    std::string username;
    std::getline(std::cin, username);

    std::cout << "Type password: ";

    std::string password;
    std::getline(std::cin, password);

    cas::HttpClientRequest request;
    request.path = "/login";
    request.headers["Username"] = username;
    request.headers["Password"] = password;

    auto response = client.post_async(request).get();

    std::cout << response.to_string() << std::endl;

    if (response.statusCode != 200)
    {
        std::cerr << "Did not successfully send message." << std::endl;
    }
    else
    {
        bearer = response.headers["Authorization"];
    }
}

void logout(cas::HttpClient& client)
{
    std::cout << "Type username: ";

    cas::HttpClientRequest request;
    request.path = "/logout";
    request.headers["Authorization"] = bearer;

    auto response = client.post_async(request).get();

    std::cout << response.to_string() << std::endl;

    if (response.statusCode != 200)
    {
        std::cerr << "Did not successfully send message." << std::endl;
    }
}

void make_get_request(cas::HttpClient& client)
{
    cas::HttpClientRequest request;
    request.headers["Authorization"] = bearer;

    auto response = client.get_async(request).get();

    std::cout << response.to_string() << std::endl;

    if (response.statusCode != 200)
    {
        std::cerr << "Did not successfully send message." << std::endl;
    }
}

int main(int argc, char** argv)
{
    cas::HttpClient client("127.0.0.1", 60001, DEFAULT_CLIENT_BUFFER_SIZE);
    
    while (true)
    {
        std::cout << "1: sign up" << std::endl;
        std::cout << "2: log in" << std::endl;
        std::cout << "3: log out" << std::endl;
        std::cout << "4: make GET request" << std::endl;
        std::cout << "Pick an option: " << std::endl;
        
        std::string optionStr;
        int option = 0;
        std::getline(std::cin, optionStr);

        try 
        {
            option = std::stoi(optionStr);
        }
        catch (const std::exception& ex)
        {

        }

        switch (option)
        {
        case 1: signup(client);
            break;
        case 2: login(client);
            break;
        case 3: logout(client);
            break;
        case 4: make_get_request(client);
            break;
        }
    }
}
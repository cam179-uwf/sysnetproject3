#include <iostream>
#include <string>

#include "cas/libs/http-client.hpp"

cas::HttpClient g_Client("127.0.0.1", 60001, DEFAULT_CLIENT_BUFFER_SIZE);
std::string g_Bearer;

void signup()
{
    std::cout << "Type username: ";

    std::string username;
    std::getline(std::cin, username);

    std::cout << "Type password: ";

    std::string password;
    std::getline(std::cin, password);

    cas::HttpRequest request;
    request.path = "/signup";
    request.headers["Username"] = username;
    request.headers["Password"] = password;

    auto response = g_Client.post_async(request).get();

    std::cout << "\n" << response.body << "\n" << std::endl;

    if (response.statusCode == 200)
    {
        g_Bearer = response.headers["Authorization"];
    }
}

void login()
{
    std::cout << "Type username: ";

    std::string username;
    std::getline(std::cin, username);

    std::cout << "Type password: ";

    std::string password;
    std::getline(std::cin, password);

    cas::HttpRequest request;
    request.path = "/login";
    request.headers["Username"] = username;
    request.headers["Password"] = password;

    auto response = g_Client.post_async(request).get();

    std::cout << "\n" << response.body << "\n" << std::endl;

    if (response.statusCode == 200)
    {
        g_Bearer = response.headers["Authorization"];
    }
}

void logout()
{
    std::cout << "Type username: ";

    cas::HttpRequest request;
    request.path = "/logout";
    request.headers["Authorization"] = g_Bearer;

    auto response = g_Client.post_async(request).get();

    std::cout << "\n" << response.body << "\n" << std::endl;
}

void change_password()
{
    std::cout << "Type old password: ";
    
    std::string oldpassword;
    std::getline(std::cin, oldpassword);

    std::cout << "Type new password: ";

    std::string newpassword;
    std::getline(std::cin, newpassword);

    cas::HttpRequest request;
    request.path = "/changepassword";
    request.headers["Authorization"] = g_Bearer;
    request.headers["oldpassword"] = oldpassword;
    request.headers["newpassword"] = newpassword;

    auto response = g_Client.post_async(request).get();

    std::cout << "\n" << response.body << "\n" << std::endl;
}

void subscribe()
{
    std::cout << "Type a location: ";
    
    std::string location;
    std::getline(std::cin, location);

    cas::HttpRequest request;
    request.path = "/subscribe";
    request.headers["Authorization"] = g_Bearer;
    request.headers["location"] = location;

    auto response = g_Client.post_async(request).get();

    std::cout << "\n" << response.body << "\n" << std::endl;
}

void unsubscribe()
{
    std::cout << "Type a location: ";
    
    std::string location;
    std::getline(std::cin, location);

    cas::HttpRequest request;
    request.path = "/unsubscribe";
    request.headers["Authorization"] = g_Bearer;
    request.headers["location"] = location;

    auto response = g_Client.post_async(request).get();

    std::cout << "\n" << response.body << "\n" << std::endl;
}

void get_my_locations()
{
    cas::HttpRequest request;
    request.path = "/getlocations";
    request.headers["Authorization"] = g_Bearer;

    auto response = g_Client.get_async(request).get();

    std::cout << "\n" << response.body << "\n" << std::endl;
}

bool is_logged_in()
{
    cas::HttpRequest request;
    request.path = "/amiloggedin";
    request.headers["Authorization"] = g_Bearer;

    auto response = g_Client.get_async(request).get();

    return response.statusCode == 200;
}

int main(int argc, char **argv)
{
    while (true)
    {
        try 
        {
            if (g_Bearer != "" && is_logged_in())
            {
                std::cout << "1: log out" << std::endl;
                std::cout << "2: change password" << std::endl;
                std::cout << "3: subscribe to a location" << std::endl;
                std::cout << "4: unsubscribe from a location" << std::endl;
                std::cout << "5: get locations" << std::endl;
                std::cout << "Pick an option or type (exit): ";

                std::string optionStr;
                int option = 0;
                std::getline(std::cin, optionStr);

                if (optionStr == "exit") break;

                try
                {
                    option = std::stoi(optionStr);
                }
                catch (const std::exception &ex) { }

                switch (option)
                {
                case 1: logout();
                    break;
                case 2: change_password();
                    break;
                case 3: subscribe();
                    break;
                case 4: unsubscribe();
                    break;
                case 5: get_my_locations();
                    break;
                }
            }
            else
            {
                std::cout << "1: register" << std::endl;
                std::cout << "2: log in" << std::endl;
                std::cout << "Pick an option or type (exit): ";

                std::string optionStr;
                int option = 0;
                std::getline(std::cin, optionStr);

                if (optionStr == "exit") break;

                try
                {
                    option = std::stoi(optionStr);
                }
                catch (const std::exception &ex) { }

                switch (option)
                {
                case 1: signup();
                    break;
                case 2: login();
                    break;
                }
            }
        }
        catch (const std::exception& ex)
        {
            std::cerr << ex.what() << std::endl;
        }
    }
}
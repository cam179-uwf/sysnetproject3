#include <iostream>
#include <string>

#include "cas/libs/http-client.hpp"

std::string bearer;

void signup(cas::HttpClient &client)
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

void login(cas::HttpClient &client)
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

void logout(cas::HttpClient &client)
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

void change_password(cas::HttpClient &client)
{
    std::cout << "Type old password: ";
    
    std::string oldpassword;
    std::getline(std::cin, oldpassword);

    std::cout << "Type new password: ";

    std::string newpassword;
    std::getline(std::cin, newpassword);

    cas::HttpClientRequest request;
    request.path = "/changepassword";
    request.headers["Authorization"] = bearer;
    request.headers["oldpassword"] = oldpassword;
    request.headers["newpassword"] = newpassword;

     auto response = client.post_async(request).get();

    std::cout << response.to_string() << std::endl;

}


void subscribe(cas::HttpClient &client)
{
    std::cout << "Type a location: ";
    
    std::string location;
    std::getline(std::cin, location);

    cas::HttpClientRequest request;
    request.path = "/subscribe";
    request.headers["Authorization"] = bearer;
    request.headers["location"] = location;

     auto response = client.post_async(request).get();

    std::cout << response.to_string() << std::endl;

}


void unsubscribe(cas::HttpClient &client)
{
    std::cout << "Type a location: ";
    
    std::string location;
    std::getline(std::cin, location);

    cas::HttpClientRequest request;
    request.path = "/unsubscribe";
    request.headers["Authorization"] = bearer;
    request.headers["location"] = location;

     auto response = client.post_async(request).get();

    std::cout << response.to_string() << std::endl;

}

void get_my_locations(cas::HttpClient &client)
{

    cas::HttpClientRequest request;
    request.path = "/getlocations";
    request.headers["Authorization"] = bearer;

     auto response = client.get_async(request).get();

    std::cout << response.to_string() << std::endl;

}

bool is_logged_in(cas::HttpClient &client)
{
    cas::HttpClientRequest request;
    request.path = "/amiloggedin";
    request.headers["Authorization"] = bearer;

    auto response = client.get_async(request).get();

    return response.statusCode == 200;
}

int main(int argc, char **argv)
{
    cas::HttpClient client("127.0.0.1", 60001, DEFAULT_CLIENT_BUFFER_SIZE);

    while (true)
    {
        try 
        {
            if (is_logged_in(client))
            {
                std::cout << "1: log out" << std::endl;
                std::cout << "2: change password" << std::endl;
                std::cout << "3: subscribe to a location" << std::endl;
                std::cout << "4: unsubscribe from a location" << std::endl;
                std::cout << "5: get locations" << std::endl;
                std::cout << "Pick an option: " << std::endl;

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
                case 1: logout(client);
                    break;
                case 2: change_password(client);
                    break;
                case 3: subscribe(client);
                    break;
                case 4: unsubscribe(client);
                    break;
                case 5: get_my_locations(client);
                    break;
                }
            }
            else
            {
                std::cout << "1: sign up" << std::endl;
                std::cout << "2: log in" << std::endl;
                std::cout << "Pick an option: " << std::endl;

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
                case 1: signup(client);
                    break;
                case 2: login(client);
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
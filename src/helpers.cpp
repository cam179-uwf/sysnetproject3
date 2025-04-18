/**
 * System & Networks II (COP4635)
 * Christian Marcellino, Ryan Waddington
 * 4/15/2025
 * 
 * For helper methods used for the server/client.
 */

#include "../libs/helpers.hpp"
#include "../libs/user-info.hpp"
#include "../cas/libs/string-helpers.hpp"

#include <chrono>
#include <random>
#include <sstream>
#include <fstream>

static const std::string base64_chars =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

std::string base64_encode(const std::string &in)
{
    std::string out;
    int val = 0, valb = -6;

    for (auto c : in) 
    {
        val = (val << 8) + c;
        valb += 8;

        while (valb >= 0) 
        {
            out.push_back(base64_chars[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    
    if (valb > -6)
    {
        out.push_back(base64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
    }

    while (out.size() % 4) 
    {
        out.push_back('=');
    }

    return out;
}

/// @brief Generates a Bearer token using a user's username.
/// @param username 
/// @return 
std::string generate_token(const std::string& username)
{
    using namespace std::chrono;

    auto now = system_clock::now().time_since_epoch();
    auto micros = duration_cast<microseconds>(now).count();

    std::mt19937_64 rng(std::random_device{}());
    std::uniform_int_distribution<uint64_t> dist;

    std::ostringstream oss;
    oss << std::hex << username << micros << dist(rng);
    return oss.str();
}

bool is_whitespace(const std::string &str)
{
    for (auto c : str)
    {
        if (!std::iswspace(c))
        {
            return false;
        }
    }

    return true;
} 

/// @brief
/// @param path 
/// @return
/// @throws std::runtime_error
std::string read_file_contents(const std::string& path)
{
    std::ifstream ifs(path);
    std::ostringstream oss;
    
    if (ifs.is_open())
    {
        std::string line;
        while (std::getline(ifs, line))
        {
            oss << line;
        }

        return oss.str();
    }
    else
    {
        throw std::runtime_error("Could not open " + path + ".");
    }

    throw std::runtime_error("File Not Found.");
}

void save_file(std::vector<UserInfo>& users)
{
    using namespace std;
    ofstream oss("users.txt");

    for (auto user : users)
    {
        oss << user.username << "," << user.password << endl;
    }
    oss.close();
}

void load_file(std::vector<UserInfo>& users)
{
    using namespace std;
    users.clear();
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
            users.push_back(userinfo);
        }
        iss.close();
    }
}
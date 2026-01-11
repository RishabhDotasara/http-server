#include "middlewares.hpp"

// --- URL Decoder Middleware
void urlDecode(Request &req, Response &res, Next next)
{
    std::string decodedString;

    for (int i = 0; i < req.data.path.size(); i++)
    {
        char x = req.data.path[i];

        if (x == '+')
            decodedString += " ";
        else if (x == '%' && i + 2 < req.data.path.size())
        {
            std::string hex = req.data.path.substr(i+1, 2);
            int ascii = std::stoi(hex, nullptr, 16);
            decodedString += static_cast<char>(ascii);

            i += 2;
        }
        else {
            decodedString += x;
        }
    }

    req.data.path = decodedString;
    next();
}
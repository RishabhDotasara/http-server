#include "middlewares.hpp"
#include "json.hpp"

using json = nlohmann::json;

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

void paramExtractor(Request &req, Response &res, Next next){
    size_t qm = req.data.path.find('?');

    if (qm == req.data.path.npos) {
        next(); 
        return;
    }

    std::string queryString = req.data.path.substr(qm+1, req.data.path.size() - qm - 1);
    size_t ampPos = queryString.find_first_of('&', 0);
    
    // update the path without the query param 
    req.data.path  = req.data.path.substr(0, qm);

    int i = 0;
    while ( i < queryString.length()){
        char x = queryString[i]; 
        // std::cout << x << "\n";
        
        std::string key; 
        std::string value; 
        
        // find the next & : key=value&key=value
        ampPos = queryString.find_first_of('&', i);

        if (ampPos == std::string::npos) ampPos = queryString.length() - 1;


        size_t eql = queryString.find_first_of('=', i);

        key = queryString.substr(i, eql - i); 
        value = queryString.substr(eql+1, ampPos - eql - 1);

        req.data.queryParams[key] = value;

        // std::cout << key << ":" << value << "\n";

        i = ampPos + 1; 
    }


    
    next();
}

void parseJson(Request &req, Response &res, Next next){
    // this converts the body attribute in a json object. 

    // only parse if the body is json 
    // std::cout << req.data.body << "\n";
    if (req.data.headers["Content-Type"] != "application/json") 
    {
        next();
        return;
    }

    req.data.bodyJson = json::parse(req.data.body);
    next();
}

void parseCookies(Request &req, Response &res, Next next){
    
}
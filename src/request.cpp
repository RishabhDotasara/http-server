#include "request.hpp"
#include <sstream>

Request::Request(int connfd):connfd(connfd){
    // on receiving the data we will parse it and store in the 
    data.bodyJson = {};
    parseRequest();
};

void Request::parseCookies(std::string cookieStr){
    size_t colon = cookieStr.find_first_of(':'); 
    cookieStr = cookieStr.substr(colon + 1, cookieStr.length() - colon - 1); 

    uint i = 0; 
    while (i < cookieStr.length()){
        size_t semicol = cookieStr.find_first_of(';', i); 

        if (semicol == std::string::npos){
            semicol = cookieStr.length();
        }

        size_t eqls = cookieStr.find_first_of('=', i);

        std::string key = cookieStr.substr(i, eqls - i); 
        std::string value = cookieStr.substr(eqls+1, semicol - eqls - 1); 

        data.cookies[key] = value; 

        i = semicol + 1; 
    };
}

void Request::parseRequest()
{
    // this function is responsible to parse the request we get from the client, so that from then we can support sending files based on the URL that the client gives.
    char buffer[1024];
    ssize_t bytes = recv(connfd, buffer, sizeof(buffer), 0);

    if (bytes > 0)
    {
        std::string request(buffer, bytes);
        std::istringstream stream(request);
        std::string line;

        // read first line
        if (std::getline(stream, line))
        {
            if (!line.empty() && line.back() == '\r')
            {
                line.pop_back();
            }

            std::istringstream lineStream(line);
            lineStream >> data.method >> data.path >> data.version;
        }

        // now read other header lines
        while (std::getline(stream, line))
        {

            if (!line.empty() && line.back() == '\r')
            {
                line.pop_back();
            }

            if (line.empty())
                break;

            size_t colon = line.find(":");
            std::string key = line.substr(0, colon);
            std::string value = line.substr(colon + 1, line.size() - colon - 1);
            data.headers[key] = value;
        }

        // parse the cookies here 
        if (!data.headers["Cookie"].empty()) parseCookies(data.headers["Cookie"]);
        
        // now the next all bytes are just body
        std::string body;
        std::getline(stream, body, '\0');
        data.body = body;
    }
}
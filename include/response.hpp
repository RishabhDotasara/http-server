#pragma once
#include <iostream> 
#include <map>
#include <fstream>
#include <sys/socket.h>

class Response{
    public: 
        int connfd;
        std::string base_path{"public"};
        std::string notFoundPath{"public/404.html"};
        std::string status{"200 OK"};
        std::map<int, std::string> STATUSES;
        
        std::map<std::string, std::string> headers;
        std::string body{""};

        Response(int connfd);
        ~Response();
        void sendFile(std::string &filepath, int statusCode=200);
        void sendHTML(std::string html, int statusCode=200);
        void setHTTPHeader(std::string contentType, std::string ContentLength);
        std::string prepareRequest(); 
        std::string getContentType(const std::string &filepath);
};
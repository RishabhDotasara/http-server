#pragma once
#include <iostream> 
#include <unordered_map>
#include <fstream>
#include <sys/socket.h>

class Response{
    public: 
        int connfd;
        std::string base_path{"public"};
        std::string notFoundPath{"public/404.html"};
        std::unordered_map<std::string, std::string> STATUSES;

        Response(int connfd);
        ~Response();
        void sendFile(std::string &filepath);
        void sendHTML(std::string html);
        std::string getHTTPHeaders(std::string status, std::string contentType, std::string ContentLength);
        std::string getContentType(const std::string &filepath);
};
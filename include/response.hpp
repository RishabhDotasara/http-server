#pragma once
#include <iostream> 
#include <map>
#include <fstream>
#include <sys/socket.h>
#include <chrono>


struct cookieOptions{
    bool HttpOnly; 
    bool Secure; 
    std::string SameSite;
    uint Max_Age;
    std::string Path;
    std::string Domain; 
    std::string Priority; 
    bool Partitioned;
    uint Expiry_days_from_now; 
    
};

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
        void setCookie(std::string key, std::string value, cookieOptions options);
        std::string prepareRequest(); 
        std::string getContentType(const std::string &filepath);
};
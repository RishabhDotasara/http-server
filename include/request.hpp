#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <fstream>
#include <functional>

struct RequestBuffer
{
    std::string method;
    std::string path;
    std::string version;
    std::unordered_map<std::string, std::string> headers;
    std::string body;
};

class Request{
    public:
        RequestBuffer data;
        std::string recvData; 
        int connfd; 
        Request(int connfd);
        void parseRequest();
};
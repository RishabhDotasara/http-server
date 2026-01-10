#pragma once 
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
#include "request.hpp"
#include "response.hpp"

class Server{
    public:
        int NOT{4}; 
        int PORT{3000};
        std::unordered_map<std::string, std::function<void(Request&, Response&)>> pathMap;

        Server(int NOT, int PORT); 
        ~Server(); 

        void start();
        static void worker(std::vector<int> &conns, Server *server);
};
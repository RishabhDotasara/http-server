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

class Server{
    public:
        int NOT{4}; 
        Server(int NOT); 
        ~Server(); 

        void setup();
        void start();
        static void worker(std::vector<int> &conns);
};
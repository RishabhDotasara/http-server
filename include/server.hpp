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
#include <map>

class Server
{
public:
    int NOT{4};
    int PORT{3000};
    std::map<std::pair<std::string, std::string>, std::function<void(Request &, Response &)>> pathMap;

    Server(int NOT, int PORT);
    ~Server();

    void start();
    static void worker(std::vector<int> &conns, Server *server);
    void registerRoute(std::string route, std::string method, std::function<void(Request &, Response &)>);
    void get(std::string route, std::function<void(Request &req, Response &res)> callback);
    void post(std::string route, std::function<void(Request &req, Response &res)> callback);
    void put(std::string route, std::function<void(Request &req, Response &res)> callback);
    void patch(std::string route, std::function<void(Request &req, Response &res)> callback);
    void del(std::string route, std::function<void(Request &req, Response &res)> callback);
};
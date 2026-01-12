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
#include <vector>

struct CorsConfig
{
    std::string origins;
    std::string methods;
    std::string headers;
};

using Next = std::function<void()>;
using Middleware = std::function<void(Request &, Response &, Next)>;

class Server
{
public:
    int NOT{4};
    int PORT{3000};
    int REQUEST_BODY_SIZE_LIMIT{8092}; //8 KB
    std::map<std::pair<std::string, std::string>, std::function<void(Request &, Response &)>> pathMap;
    std::vector<Middleware> middlewares;
    std::map<std::string, std::string> CORS;



    Server(int NOT, int PORT);
    ~Server();

    void start();
    static void worker(std::vector<int> &conns, Server *server);
    void registerRoute(std::string route, std::string method, std::function<void(Request &, Response &)>);
    void setCors(CorsConfig corsConfig);
    void use(Middleware func);

    void get(std::string route, std::function<void(Request &req, Response &res)> callback);
    void post(std::string route, std::function<void(Request &req, Response &res)> callback);
    void put(std::string route, std::function<void(Request &req, Response &res)> callback);
    void patch(std::string route, std::function<void(Request &req, Response &res)> callback);
    void del(std::string route, std::function<void(Request &req, Response &res)> callback);
};
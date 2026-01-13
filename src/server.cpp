#include "http.hpp"
#include "server.hpp"
#include "request.hpp"
#include "response.hpp"
#include "logger.hpp"
#include <filesystem>
#include <arpa/inet.h>

std::mutex mtx;
std::condition_variable cv;

Server::Server(int NOT, int PORT)
{
    this->NOT = NOT;
    this->PORT = PORT;
    CORS["Access-Control-Allow-Origin"] = "";
    CORS["Access-Control-Allow-Methods"] = "";
    CORS["Access-Control-Allow-Headers"] = "";

    // for all the files in pucliv folder, already create the paths for all of them
    namespace fs = std::filesystem;

    for (const auto &entry : fs::directory_iterator("public"))
    {
        if (entry.is_regular_file())
        {
            std::string filename = entry.path().filename().string();
            std::string route = "/public/" + filename;

            // Register route
            pathMap[{route, "GET"}] = [filename](Request &req, Response &res)
            {
                std::string filepath = "public/" + filename;
                res.sendFile(filepath, 200);
            };

            logger.debug("Auto-registered route: " + route + " -> " + filename);
        }
    }
};

Server::~Server() {};

void Server::worker(std::vector<int> &conns, Server *server)
{

    while (true)
    {
        int connfd;

        {
            std::unique_lock<std::mutex> lock(mtx);

            cv.wait(lock, [&conns]
                    { return conns.size() > 0; });

            // std::cout << "Thread " << std::this_thread::get_id()
            //           << " handling request\n"
            //           << std::flush;

            connfd = conns.back();
            conns.pop_back();
        }

        // -- We have the connection, use that to get the IP
        sockaddr_in peeraddr;
        socklen_t peeraddr_len = sizeof(peeraddr);
        getpeername(connfd, (sockaddr *)&peeraddr, &peeraddr_len);
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(peeraddr.sin_addr), ip, INET_ADDRSTRLEN);
        struct timeval timeout;
        timeout.tv_sec = server->CONNECTION_TIMEOUT; // 5 seconds
        timeout.tv_usec = 0;
        setsockopt(connfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

        int requestCount{0}; 
        time_t start_timestamp = std::time(nullptr); 
        time_t rolling_timestamp = std::time(nullptr);

        // loop to keep connections as supported in HTTP/1.1
        while (requestCount <= server->CONNECTION_MAX_REQUESTS)
        {

            // request buffer
            Request request{connfd};
            Response response{connfd};
            requestCount++;

            // so rolling timestamp checks the time of inactivity on the connection 
            if (request.data.method.empty()) {
                rolling_timestamp = std::time(nullptr);
                // close(connfd); 
                break;
            }
            
            


            // -- Set IP in the request
            std::string req_ip{ip, INET_ADDRSTRLEN};
            request.data.ip = req_ip;

            auto rateLimit_it = server->rateLimitBucket.find(req_ip);
            std::time_t now = std::time(nullptr);

            if (server->RateLimitEnabled)
            {
                if ((rateLimit_it != server->rateLimitBucket.end()))
                {
                    if (rateLimit_it->second.second <= 0 && now - rateLimit_it->second.first < server->REQUEST_LIMIT_WINDOW)
                    {
                        response.sendHTML("", 429);
                        continue;
                    }
                    else if (rateLimit_it->second.second <= 0 && now - rateLimit_it->second.first >= server->REQUEST_LIMIT_WINDOW)
                    {
                        rateLimit_it->second.second = server->REQUEST_LIMIT;
                        logger.debug("Rate limit window reset for IP: " + req_ip);
                    }
                    else
                    {
                        rateLimit_it->second.second -= 1;
                    }
                }
                else
                {
                    server->rateLimitBucket[req_ip] = {now, server->REQUEST_LIMIT};
                }
            }

            // ---- CHECK REQUEST BODY SIZE
            if (!request.data.headers["Content-Length"].empty() && std::stoi(request.data.headers["Content-Length"]) > server->REQUEST_BODY_SIZE_LIMIT)
            {
                response.sendHTML("", 413);
                continue;
            }

            // ---- CORS SETUP -----
            // so if we get a OPTIONS request, send the response with some set headers.

            // apply cors headers to all responses
            for (auto &it : server->CORS)
            {
                response.setHTTPHeader(it.first, it.second);
            }

            if (request.data.method == "OPTIONS")
            {
                response.sendHTML("", 204);
                continue;
            }

            // ---- Middleware execution before the main handler
            {
                bool executeNext = true;

                for (Middleware func : server->middlewares)
                {
                    executeNext = false;

                    func(request, response, [&executeNext]()
                         { executeNext = true; });

                    if (!executeNext)
                        break;
                }

                // if the last middleware didnt call next, we just leave the request there
                if (!executeNext)
                    continue;
            }

            // ---- Route Matching ----
            auto it = server->pathMap.find({request.data.path, request.data.method});
            bool routeExists = false;
            bool dynamicParams = false;
            size_t colonForDP = request.data.path.find_first_of(":");
            if (colonForDP != std::string::npos)
                dynamicParams = true;

            // fill in the params becfore the function execution

            for (auto &it : server->pathMap)
            {
                bool dynamicRoute = false;
                std::string method = it.first.second;
                std::string route = it.first.first;
                size_t colon = route.find_first_of(":");

                // dynamic route check
                if (colon != std::string::npos)
                    dynamicRoute = true;

                // /usr/:id/role/:role
                // /usr/2/role/admin
                if (dynamicRoute)
                {

                    // ---Check the path and the method
                    if (request.data.path.substr(0, colon - 1) != route.substr(0, colon - 1))
                        continue;
                    if (request.data.method != it.first.second)
                        continue;

                    // --- PARAM Extraction

                    int i = colon; // path
                    int j = colon; // route
                    while (i < request.data.path.length() && j < route.length())
                    {
                        size_t nextSlashInPath = request.data.path.find_first_of("/", i);
                        size_t nextSlashInRoute = route.find_first_of("/", j);

                        if (nextSlashInRoute == std::string::npos)
                        {
                            // nextSlashInPath = request.data.path.length();
                            nextSlashInRoute = route.length();
                        }

                        // check if this one is a param or simple route
                        if (route[j] != ':' && request.data.path.substr(i, nextSlashInPath - i) == route.substr(j, nextSlashInRoute - j))
                        {
                            i = nextSlashInPath + 1;
                            j = nextSlashInRoute + 1;
                            continue;
                        }

                        // if (nextSlashInPath == std::string::npos){
                        //     nextSlashInPath = request.data.path.length();
                        // }

                        std::string value = request.data.path.substr(i, nextSlashInPath - i);
                        std::string key = route.substr(j + 1, nextSlashInRoute - j - 1);
                        request.data.params[key] = value;

                        // std::cout << "[DEBUG] Extracted param: " << key << " = " << value << "\n";

                        i = nextSlashInPath + 1;
                        j = nextSlashInRoute + 1;
                    }

                    // ---- Function Calling
                    it.second(request, response);
                }
                else
                {
                    // just check if the route matches
                    if (request.data.path == route && request.data.method == it.first.second)
                        it.second(request, response);
                    else
                        continue;
                }
            }

            logger.request(request.data.method, request.data.path, response.status);

            // if connection set to close, just close it else just keep reading
            if (request.data.headers["Connection"] == "close")
                rolling_timestamp = start_timestamp + server->CONNECTION_TIMEOUT + 1;

        }
        
        // so when you are out of the loop, it simply means this connection needs to be closed 
        close(connfd);
        logger.debug("Closing the Connection for IP: " + std::string(ip));
    
    }

}

void Server::start()
{

    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0)
    {
        logger.fatal("Failed to create socket");
        exit(1);
    }
    logger.info("Socket created successfully");

    // to reuse the port
    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        logger.fatal("Failed to set socket options (SO_REUSEADDR)");
        exit(1);
    }
    logger.debug("Socket option SO_REUSEADDR enabled");

    sockaddr_in sock_addr{};
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(PORT);
    sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(server_socket, (sockaddr *)&sock_addr, sizeof(sock_addr)) < 0)
    {
        logger.fatal("Failed to bind socket to port " + std::to_string(PORT));
        exit(1);
    }

    logger.info("Server listening on port " + std::to_string(PORT));

    listen(server_socket, 5);

    sockaddr_in peer_addr{};
    socklen_t peer_addr_len = sizeof(peer_addr);

    // initialise the thread pool

    int N = NOT;

    std::vector<int> conns;

    // start the threads
    for (int i = 0; i < N; i++)
    {
        std::thread t(worker, std::ref(conns), this);
        t.detach();
    }
    logger.info("Thread pool initialized with " + std::to_string(N) + " workers");

    logger.info("Server ready - accepting connections");
    while (true)
    {

        int connfd = accept(server_socket, (sockaddr *)&peer_addr, &peer_addr_len);
        if (connfd < 0)
        {
            logger.error("Failed to accept connection");
            continue;
        }

        std::lock_guard<std::mutex> lock(mtx);
        conns.push_back(connfd);
        cv.notify_one();
    } 
}

void Server::registerRoute(std::string route, std::string method, std::function<void(Request &, Response &)> callback)
{
    // check if the route is dynamic or not  
    this->pathMap[{route, method}] = callback;
}

void Server::setCors(CorsConfig corsConfig)
{
    CORS["Access-Control-Allow-Origin"] = corsConfig.origins;
    CORS["Access-Control-Allow-Methods"] = corsConfig.methods;
    CORS["Access-Control-Allow-Headers"] = corsConfig.headers;
    CORS["Access-Control-Max-Age"] = "86400";
}

void Server::use(Middleware func)
{
    middlewares.push_back(func);
}

void Server::get(std::string route, std::function<void(Request &req, Response &res)> callback)
{
    this->registerRoute(route, "GET", callback);
}

void Server::post(std::string route, std::function<void(Request &req, Response &res)> callback)
{
    this->registerRoute(route, "POST", callback);
}

void Server::put(std::string route, std::function<void(Request &req, Response &res)> callback)
{
    this->registerRoute(route, "PUT", callback);
}

void Server::patch(std::string route, std::function<void(Request &req, Response &res)> callback)
{
    this->registerRoute(route, "PATCH", callback);
}

void Server::del(std::string route, std::function<void(Request &req, Response &res)> callback)
{
    this->registerRoute(route, "DELETE", callback);
}

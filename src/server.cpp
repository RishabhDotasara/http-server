#include "http.hpp"
#include "server.hpp"
#include "request.hpp"
#include "response.hpp"
#include <filesystem>

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

            std::cout << "[INFO] Registered route: " << route << " -> " << filename << "\n";
        }
    }
};
Server::~Server() {};

// --- Helper function for the next() call in middlewares

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

        // request buffer
        Request request{connfd};
        Response response{connfd};

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
            close(connfd);
            continue;
        }

        // ---- Middleware execution before the main handler
        {
            bool executeNext = true;
           
            for (Middleware func : server->middlewares)
            {
                executeNext = false;
                
                func(request, response, [&executeNext](){
                    executeNext = true;
                });

                if (!executeNext) break;
            }

            // if the last middleware didnt call next, we just leave the request there
            if (!executeNext) continue;
        }

        // call the particular mapped function in here
        // ---- Route Matching ----
        auto it = server->pathMap.find({request.data.path, request.data.method});
        bool routeExists = false;

        for (const auto &it : server->pathMap)
        {
            if (it.first.first == request.data.path)
                routeExists = true;
        }

        if (it != server->pathMap.end())
        {
            it->second(request, response);
        }
        else if (routeExists)
        {
            // return 405
            response.sendHTML("<h1>Method Not Allowed!</h1>", 405);
        }
        else
        {
            std::string filepath = "public/404.html";
            response.sendFile(filepath, 404);
        }

        std::cout << "[REQUEST] " << request.data.method << " " << request.data.path << " " << response.status << "\n"
                  << std::flush;

        close(connfd);
    }
}

void Server::start()
{

    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0)
    {
        perror("socket");
        exit(1);
    }
    std::cout << "[INFO] Socket Setup Complete!\n";

    // to reuse the port
    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("setsockopt");
        exit(1);
    }

    sockaddr_in sock_addr{};
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(PORT);
    sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(server_socket, (sockaddr *)&sock_addr, sizeof(sock_addr)) < 0)
    {
        perror("bind");
    }

    std::cout << "[INFO] Listening at PORT " + std::to_string(PORT) << "\n";

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

    std::cout << "[INFO] Accepting Connections from clients!\n";
    while (true)
    {

        int connfd = accept(server_socket, (sockaddr *)&peer_addr, &peer_addr_len);
        if (connfd < 0)
        {
            perror("accept");
            continue;
        }

        std::lock_guard<std::mutex> lock(mtx);
        conns.push_back(connfd);
        cv.notify_one();
    }
}

void Server::registerRoute(std::string route, std::string method, std::function<void(Request &, Response &)> callback)
{
    this->pathMap[{route, method}] = callback;
}

void Server::setCors(CorsConfig corsConfig)
{
    CORS["Access-Control-Allow-Origin"] = corsConfig.origins;
    CORS["Access-Control-Allow-Methods"] = corsConfig.methods;
    CORS["Access-Control-Allow-Headers"] = corsConfig.headers;
    CORS["Access-Control-Max-Age"] = "86400";
}

void Server::use(Middleware func){
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

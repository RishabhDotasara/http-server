#include "server.hpp"

int main()
{
    Server server(4, 3000);

    // ----- CORS Polciy Setup
    CorsConfig config;
    config.origins = "*";
    config.methods = "GET, POST, PUT, PATCH";
    config.headers = "Content-Type, Authorization";
    server.setCors(config);

    // ---- ROUTES ----
    server.get("/index", [](Request &req, Response &res){
        std::string filepath = "public/index.html";
        res.sendFile(filepath);
    });
               

    server.post("/index", [](Request &req, Response &res)
                { res.sendHTML("<h1>This is the GET route.</h1>", 200); });

    server.start();
    return 0;
}

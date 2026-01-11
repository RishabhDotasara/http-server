#include "server.hpp"
#include "middlewares.hpp"
#include "json.hpp"

using json = nlohmann::json;

int main()
{
    Server server(4, 3000);

    // ----- CORS Polciy Setup
    CorsConfig config;
    config.origins = "*";
    config.methods = "GET, POST, PUT, PATCH";
    config.headers = "Content-Type, Authorization";
    server.setCors(config);

    // ----- Add all middlewares here ---
    server.use(urlDecode);
    server.use(paramExtractor);
    server.use(parseJson);

    // ---- ROUTES ----
    server.get("/index", [](Request &req, Response &res){
        std::string filepath = "public/index.html";
        std::string SameSite{"Lax"};
        std::string Path{"/index"};
        uint Max_Age = 1000;
        res.setCookie("testCookie", "testData", {true, true,SameSite , Max_Age, Path});
        res.sendFile(filepath); 
    });

    server.post("/index", [](Request &req, Response &res){
        res.sendHTML("<h1>This is the GET route.</h1>", 200);
        // std::cout << req.data.bodyJson << "\n";
    });

    server.start();
    return 0;
}

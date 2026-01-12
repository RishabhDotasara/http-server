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
        cookieOptions config;
        config.Domain = "localhost.com"; 
        config.Expiry_days_from_now = 5; 
        config.HttpOnly = true; 
        config.Max_Age = 3000; 
        config.Path = "/index"; 
        config.Priority = "Medium"; 
        config.SameSite = "Lax"; 
        config.Secure = false;
        
        res.setCookie("testCookie", "testData", config);
        res.sendFile(filepath); 
    });

    server.post("/index", [](Request &req, Response &res){
        res.sendHTML("<h1>This is the GET route.</h1>", 200);
        // std::cout << req.data.bodyJson << "\n";
    });

    server.start();
    return 0;
}

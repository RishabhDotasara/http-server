#include "server.hpp"
#include "middlewares.hpp"
#include "json.hpp"
#include "logger.hpp"

using json = nlohmann::json;

int main()
{
    // Configure logger
    LoggerConfig logConfig;
    logConfig.minLevel = LogLevel::DEBUG;
    logConfig.showTimestamp = true;
    logConfig.showColors = true;
    logConfig.logToFile = false;
    logger.configure(logConfig);

    logger.info("Initializing HTTP Server...");

    Server server(4, 3000);

    // --- Rate Limit Setting--- IP based rate limiting
    server.REQUEST_LIMIT = 100; 
    server.REQUEST_LIMIT_WINDOW = 60; //in seconds

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
    server.get("/index", [](Request &req, Response &res)
               {
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
        res.sendFile(filepath); });

    server.post("/index", [](Request &req, Response &res)
                {
                    res.sendHTML("<h1>This is the GET route.</h1>", 200);
                    // std::cout << req.data.bodyJson << "\n";
                });

    server.get("/index/:userId/:courseId", [](Request &req, Response &res)
               { res.sendHTML("<h1>You entered " + req.data.params["userId"] + " " + req.data.params["courseId"] + "</h1>", 200); });

    server.get("/usr/:userId/:courseId/:thi", [](Request &req, Response &res)
               { res.sendHTML("<h1>You entered " + req.data.params["userId"] + " " + req.data.params["courseId"] + " " + req.data.params["thi"] + " " + "</h1>", 200); });

    server.get("/api/:version/org/:orgName/team/:teamName", [](Request &req, Response &res)
               { res.sendHTML(
                     "<h2>API " + req.data.params["version"] + "</h2>"
                                                               "<p>Org: " +
                         req.data.params["orgName"] + "</p>"
                                                      "<p>Team: " +
                         req.data.params["teamName"] + "</p>",
                     200); });

    server.get("/shop/:category/product/:productName", [](Request &req, Response &res)
               { res.sendHTML(
                     "<h1>" + req.data.params["productName"] + "</h1>"
                                                               "<p>Category: " +
                         req.data.params["category"] + "</p>",
                     200); });

    server.start();
    return 0;
}

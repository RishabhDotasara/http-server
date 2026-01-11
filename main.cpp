#include "server.hpp"

int main()
{
    Server server(4, 3000);

    server.get("/index", [](Request &req, Response &res)
               { res.sendHTML("<h1>This is the GET route.</h1>", 200); });

    server.post("/index", [](Request &req, Response &res)
                { res.sendHTML("<h1>This is the GET route.</h1>", 200); });

    server.start();
    return 0;
}

#include "server.hpp"
#include "response.hpp"
#include "request.hpp"

int main() {
    Server server(4, 3000);

    server.start();
    return 0;
}

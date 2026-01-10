#include "http.hpp"
#include "server.hpp"


std::mutex mtx;
std::condition_variable cv;
HTTP http;

Server::Server(int NOT){
    NOT = NOT;
};
Server::~Server(){};

void Server::worker(std::vector<int> &conns){

    while (true){
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
        Request requestBuffer{};

        http.parseRequest(connfd, requestBuffer);
        // std::cout << "Method: " << requestBuffer.method << "\n"
        //           << "Path: " << requestBuffer.path << "\n"
        //           << "Version: " << requestBuffer.version << "\n"
        //           << "Body: " << requestBuffer.body << "\n"
        //           << std::flush;
        //           std::cout << "\n";
        http.sendFile(connfd, requestBuffer.path);
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
    sock_addr.sin_port = htons(8081);
    sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(server_socket, (sockaddr *)&sock_addr, sizeof(sock_addr)) < 0)
    {
        perror("bind");
        
    }
    std::cout << "[INFO] Port Bound to the Socket!\n";

    listen(server_socket, 5);

    sockaddr_in peer_addr{};
    socklen_t peer_addr_len = sizeof(peer_addr);


    // initialise the thread pool 
    
    int N = NOT; 
    
    std::vector<int> conns;
    
    // start the threads 
    for (int i = 0; i < N; i++){
        std::thread t(worker, std::ref(conns));
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

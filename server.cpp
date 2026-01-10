#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>

struct ThreadPool {
    std::vector<std::thread> threads;
    std::vector<int> conns; 
};

std::mutex mtx;
std::condition_variable cv;

void worker(std::vector<int> &conns){

    while (true){
        int connfd;
        { 
            std::unique_lock<std::mutex> lock(mtx);

            cv.wait(lock, [&conns]
                    { return conns.size() > 0; });

            std::cout << "Thread " << std::this_thread::get_id()
                      << " handling request\n"
                      << std::flush;

            connfd = conns.back();
            conns.pop_back();
        }

        sleep(1);
        
        std::string body = "Hello World!\n";
        std::string msg =
        "HTTP/1.1 200 OK\r\n"
        "Content-Length: " +
        std::to_string(body.size()) + "\r\n"
        "Content-Type: text/plain\r\n"
        "\r\n" +
        body;
        
        send(connfd, msg.c_str(), msg.size(), 0);
        
        close(connfd);
       
    }

}

int main()
{

    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0)
    {
        perror("socket");
        return 1;
    }
    std::cout << "Socket Setup Complete!\n";

    sockaddr_in sock_addr{};
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(8080);
    sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(server_socket, (sockaddr *)&sock_addr, sizeof(sock_addr)) < 0)
    {
        perror("bind");
        return 1;
    }
    std::cout << "Port Bound to the Socket!\n";

    listen(server_socket, 5);

    sockaddr_in peer_addr{};
    socklen_t peer_addr_len = sizeof(peer_addr);


    // initialise the thread pool 
    
    int N = 4; 

    std::vector<int> conns;
    
    // start the threads 
    for (int i = 0; i < N; i++){
        std::thread t(worker, std::ref(conns));
        t.detach();
    }
    std::cout << "Accepting Connections from clients!\n";
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

    return 0;
}

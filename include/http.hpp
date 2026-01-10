// #pragma once 

// #include <iostream> 
// #include <unordered_map>

// struct Request {
//     std::string method; 
//     std::string path;
//     std::string version; 
//     std::unordered_map<std::string, std::string> headers;
//     std::string body;
// };

// class HTTP {
//     public: 
//         std::string base_path{"public"};
//         std::string notFoundPath{"public/404.html"};
//         std::unordered_map<std::string, std::string> STATUSES; 

//         HTTP(); 
//         ~HTTP();


//         void sendFile(int connfd, std::string &filepath); 
//         void sendHTML(int connfd, std::string html);
//         void parseRequest(int connfd, Request &requestBuffer);
//         std::string getHTTPHeaders(std::string status, std::string contentType, std::string ContentLength);

// };


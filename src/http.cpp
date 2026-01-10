#include "http.hpp"
#include <fstream>
#include <sstream>
#include <sys/socket.h>

HTTP::HTTP(){

    // define all the status codes here itself 
    STATUSES["success"] = "200 OK";
    STATUSES["notFound"] = "404 Not Found"; 
    STATUSES["serverError"] = "500 Internal Server Error"; 
 
};

HTTP::~HTTP(){};

std::string HTTP::getHTTPHeaders(std::string status, std::string contentType, std::string ContentLength){
    std::string headers = 
    "HTTP/1.1 " + status + "\r\n"
    "Content-Type: " + contentType + "\r\n"
    "Content-Length: " + ContentLength + "\r\n"
    "\r\n";

    return headers;
}

void HTTP::sendFile(int connfd, std::string &filepath)
{
    // in this function we will need to store the file contents in the buffer first and then send  them through the socket.
    // open the file
    filepath = base_path + filepath;
    std::ifstream file(filepath, std::ios::binary);

    if (!file)
    {
        file.open(notFoundPath, std::ios::binary);
    }

    // get the file size first
    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);

    // now send the file in the reponse with appropriate file type
    std::string headers = getHTTPHeaders(STATUSES["success"], "text/html", std::to_string(size));

    send(connfd, headers.c_str(), headers.size(), 0);

    // we will chunk the file here as we cannot create those big buffers in memory
    char buffer[8092];
    while (file.read(buffer, sizeof(buffer)) || file.gcount() > 0)
    {
        send(connfd, buffer, file.gcount(), 0);
    }
};

void HTTP::sendHTML(int connfd, std::string html)
{
    // prepare the reponse
    std::string headers = getHTTPHeaders("success", "text/html", std::to_string(html.size())); 
    std::string response = headers + "\r\n" + html;
    // now send it in the response
    send(connfd, response.c_str(), response.size(), 0);
};


void HTTP::parseRequest(int connfd, Request &requestBuffer)
{
    // this function is responsible to parse the request we get from the client, so that from then we can support sending files based on the URL that the client gives.
    char buffer[1024];
    ssize_t bytes = recv(connfd, buffer, sizeof(buffer), 0);
    

    if (bytes > 0)
    {
        std::string request(buffer, bytes);
        std::istringstream stream(request);
        std::string line;

        // read first line 
        if (std::getline(stream, line)){
            if (!line.empty() && line.back() == '\r'){
                line.pop_back();
            }

            std::istringstream lineStream(line);
            lineStream >> requestBuffer.method >> requestBuffer.path  >> requestBuffer.version;
        }

        // now read other header lines 
        while (std::getline(stream, line)){
            
            if (!line.empty() && line.back() == '\r'){
                line.pop_back();
            }

            if (line.empty()) break;

            size_t colon = line.find(":");
            std::string key = line.substr(0, colon); 
            std::string value = line.substr(colon+1, line.size() - colon - 1);
            requestBuffer.headers[key] = value;
        }

        // now the next all bytes are just body 
        std::string body;
        std::getline(stream, body, '\0');
        requestBuffer.body = body;
    }
}
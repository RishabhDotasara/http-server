#include "response.hpp"
#include <map>

Response::Response(int connfd)
{
    this->connfd = connfd;
    // 2xx Success
    STATUSES[200] = "200 OK";
    STATUSES[201] = "201 Created";
    STATUSES[202] = "202 Accepted";
    STATUSES[204] = "204 No Content";

    // 3xx Redirection
    STATUSES[301] = "301 Moved Permanently";
    STATUSES[302] = "302 Found";
    STATUSES[304] = "304 Not Modified";
    STATUSES[307] = "307 Temporary Redirect";
    STATUSES[308] = "308 Permanent Redirect";

    // 4xx Client Errors
    STATUSES[400] = "400 Bad Request";
    STATUSES[401] = "401 Unauthorized";
    STATUSES[403] = "403 Forbidden";
    STATUSES[404] = "404 Not Found";
    STATUSES[405] = "405 Method Not Allowed";
    STATUSES[408] = "408 Request Timeout";
    STATUSES[409] = "409 Conflict";
    STATUSES[410] = "410 Gone";
    STATUSES[413] = "413 Payload Too Large";
    STATUSES[415] = "415 Unsupported Media Type";
    STATUSES[429] = "429 Too Many Requests";

    // 5xx Server Errors
    STATUSES[500] = "500 Internal Server Error";
    STATUSES[501] = "501 Not Implemented";
    STATUSES[502] = "502 Bad Gateway";
    STATUSES[503] = "503 Service Unavailable";
    STATUSES[504] = "504 Gateway Timeout";
}

Response::~Response() {};

void Response::setHTTPHeader(std::string key, std::string value){
    headers[key] = value;
}

void Response::setCookie(std::string key, std::string value, cookieOptions options){
    if (options.Secure) value += "; Secure";
    if (options.HttpOnly) value += "; HttpOnly"; 
    if (!options.SameSite.empty()) value += "; SameSite=" + options.SameSite; 
    if (options.Max_Age) value += "; Max-Age=" + std::to_string(options.Max_Age);
    if (!options.Path.empty()) value += "; Path=" + options.Path;
    setHTTPHeader("Set-Cookie", key + "=" + value);
}

std::string Response::prepareRequest(){
    std::string request = "HTTP/1.1 " + status + "\r\n"; 
    for (auto &it: headers){
        request += it.first + ": " + it.second + "\r\n";
    } 
    // --- Empty line for headers ending 
    request += "\r\n"; 
    // ---- Body Starts from here ------ 
    request += this->body; 
    return request;
}

void Response::sendFile(std::string &filepath, int statusCode)
{
    // in this function we will need to store the file contents in the buffer first and then send  them through the socket.
    // open the file
    status = STATUSES[statusCode];
    filepath = filepath;
    std::ifstream file(filepath, std::ios::binary);

    if (!file)
    {
        status = STATUSES[404];
        file.open(notFoundPath, std::ios::binary);
    }

    // get the file size first
    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);

    // now send the file in the reponse with appropriate file type
    // std::string headers = getHTTPHeaders(STATUSES[statusCode], getContentType(filepath), std::to_string(size));
    this->setHTTPHeader("Content-Type", getContentType(filepath)); 
    this->setHTTPHeader("Content-Length", std::to_string(size)); 
    std::string preparedRequest = prepareRequest();

    send(connfd, preparedRequest.c_str(), preparedRequest.size(), 0);

    // we will chunk the file here as we cannot create those big buffers in memory
    char buffer[8092];
    while (file.read(buffer, sizeof(buffer)) || file.gcount() > 0)
    {
        send(connfd, buffer, file.gcount(), 0);
    }
}

std::string Response::getContentType(const std::string &filepath)
{
    static const std::map<std::string, std::string> mimeTypes = {
        // HTML/CSS/JS
        {"html", "text/html"},
        {"htm", "text/html"},
        {"css", "text/css"},
        {"js", "application/javascript"},
        {"json", "application/json"},
        {"xml", "application/xml"},

        // Images
        {"jpg", "image/jpeg"},
        {"jpeg", "image/jpeg"},
        {"png", "image/png"},
        {"gif", "image/gif"},
        {"svg", "image/svg+xml"},
        {"webp", "image/webp"},
        {"ico", "image/x-icon"},

        // Fonts
        {"woff", "font/woff"},
        {"woff2", "font/woff2"},
        {"ttf", "font/ttf"},
        {"otf", "font/otf"},

        // Documents
        {"pdf", "application/pdf"},
        {"txt", "text/plain"},
        {"csv", "text/csv"},

        // Audio
        {"mp3", "audio/mpeg"},
        {"wav", "audio/wav"},
        {"ogg", "audio/ogg"},

        // Video
        {"mp4", "video/mp4"},
        {"webm", "video/webm"},

        // Archives
        {"zip", "application/zip"},
        {"tar", "application/x-tar"},
        {"gz", "application/gzip"}};

    size_t dot = filepath.find_last_of(".");
    if (dot == std::string::npos)
    {
        return "application/octet-stream";
    }

    std::string ext = filepath.substr(dot + 1);

    auto it = mimeTypes.find(ext);
    if (it != mimeTypes.end())
    {
        return it->second;
    }

    return "application/octet-stream";
}

void Response::sendHTML(std::string html, int statusCode)
{
    // prepare the reponse
    status = STATUSES[statusCode];
    this->setHTTPHeader("Content-Type", "text/html");
    this->setHTTPHeader("Content-Length", std::to_string(html.size()));
    this->body = html;
    std::string preparedRequest = prepareRequest();
    // now send it in the response
    send(connfd, preparedRequest.c_str(), preparedRequest.size(), 0);
};
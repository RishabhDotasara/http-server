#include "response.hpp"


Response::Response(int connfd){
    this->connfd = connfd;
    STATUSES["success"] = "200 OK";
    STATUSES["notFound"] = "404 Not Found";
    STATUSES["serverError"] = "500 Internal Server Error";
}

Response::~Response(){};

std::string Response::getHTTPHeaders(std::string status, std::string contentType, std::string ContentLength)
{
    std::string headers =
        "HTTP/1.1 " + status + "\r\n"
                               "Content-Type: " +
        contentType + "\r\n"
                      "Content-Length: " +
        ContentLength + "\r\n"
                        "\r\n";

    return headers;
}

void Response::sendFile(std::string &filepath){
    // in this function we will need to store the file contents in the buffer first and then send  them through the socket.
    // open the file
    filepath = filepath;
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
    std::string headers = getHTTPHeaders(STATUSES["success"], getContentType(filepath), std::to_string(size));

    send(connfd, headers.c_str(), headers.size(), 0);

    // we will chunk the file here as we cannot create those big buffers in memory
    char buffer[8092];
    while (file.read(buffer, sizeof(buffer)) || file.gcount() > 0)
    {
        send(connfd, buffer, file.gcount(), 0);
    }
}

std::string Response::getContentType(const std::string &filepath)
{
    static const std::unordered_map<std::string, std::string> mimeTypes = {
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

void Response::sendHTML(std::string html)
{
    // prepare the reponse
    std::string headers = getHTTPHeaders("success", "text/html", std::to_string(html.size()));
    std::string response = headers + "\r\n" + html;
    // now send it in the response
    send(connfd, response.c_str(), response.size(), 0);
};
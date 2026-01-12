#pragma once
#include <string>
#include <cstdint>

// Server configuration
struct ServerConfig
{
    uint16_t port = 3000;
    uint8_t num_threads = 4;
    int backlog = 5;
    bool reuse_address = true;
};

// Path configuration
struct PathConfig
{
    std::string public_dir = "public";
    std::string not_found_page = "public/404.html";
};

// Buffer size configuration
struct BufferConfig
{
    size_t request_buffer_size = 1024;
    size_t file_chunk_size = 8092;
    size_t cookie_timestamp_buffer = 100;
};

// CORS configuration
struct CorsDefaults
{
    bool enabled = true;
    std::string origins = "*";
    std::string methods = "GET, POST, PUT, PATCH, DELETE";
    std::string headers = "Content-Type, Authorization";
    uint32_t max_age = 86400; // 24 hours in seconds
};

// HTTP configuration
struct HttpConfig
{
    std::string version = "HTTP/1.1";
    std::string default_status = "200 OK";
    std::string default_content_type = "application/octet-stream";
};

// Cookie defaults
struct CookieDefaults
{
    std::string default_same_site = "Lax";
    uint32_t default_max_age = 3600; // 1 hour
    std::string default_path = "/";
    bool http_only = true;
    bool secure = false;
};

// Logging configuration
struct LoggingConfig
{
    bool request_logs = true;
    bool route_registration = true;
    bool thread_info = false;
};

// Auto route registration
struct AutoRouteConfig
{
    bool enabled = true;
    std::string directory = "public";
    std::string method = "GET";
    std::string route_prefix = "/public/";
};

// Main settings class combining all configs
class Settings
{
public:
    ServerConfig server;
    PathConfig paths;
    BufferConfig buffers;
    CorsDefaults cors;
    HttpConfig http;
    CookieDefaults cookies;
    LoggingConfig logging;
    AutoRouteConfig auto_routes;

    Settings(); // Constructor with defaults

    // Optional: Load from file
    void loadFromFile(const std::string &filepath);

    // Optional: Validate settings
    bool validate() const;
};
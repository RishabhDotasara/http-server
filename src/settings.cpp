#include "settings.hpp"
#include "json.hpp"
#include "logger.hpp"
#include <fstream>
#include <iostream>

using json = nlohmann::json;

Settings::Settings() {};

void Settings::loadFromFile(const std::string &filepath)
{
    std::ifstream file(filepath);
    if (!file.is_open())
    {
        logger.error("Could not open settings file: " + filepath);
        return;
    }
    logger.info("Loading settings from: " + filepath);

    json settings = json::parse(file);

    // Server config
    if (settings.contains("server"))
    {
        if (settings["server"].contains("port"))
            server.port = settings["server"]["port"];
        if (settings["server"].contains("num_threads"))
            server.num_threads = settings["server"]["num_threads"];
        if (settings["server"].contains("backlog"))
            server.backlog = settings["server"]["backlog"];
        if (settings["server"].contains("reuse_address"))
            server.reuse_address = settings["server"]["reuse_address"];
    }

    // Path config
    if (settings.contains("paths"))
    {
        if (settings["paths"].contains("public_dir"))
            paths.public_dir = settings["paths"]["public_dir"];
        if (settings["paths"].contains("not_found_page"))
            paths.not_found_page = settings["paths"]["not_found_page"];
    }

    // CORS config
    if (settings.contains("cors"))
    {
        if (settings["cors"].contains("enabled"))
            cors.enabled = settings["cors"]["enabled"];
        if (settings["cors"].contains("origins"))
            cors.origins = settings["cors"]["origins"];
        if (settings["cors"].contains("methods"))
            cors.methods = settings["cors"]["methods"];
        if (settings["cors"].contains("headers"))
            cors.headers = settings["cors"]["headers"];
        if (settings["cors"].contains("max_age"))
            cors.max_age = settings["cors"]["max_age"];
    }

    // HTTP config
    if (settings.contains("http"))
    {
        if (settings["http"].contains("version"))
            http.version = settings["http"]["version"];
        if (settings["http"].contains("default_status"))
            http.default_status = settings["http"]["default_status"];
        if (settings["http"].contains("default_content_type"))
            http.default_content_type = settings["http"]["default_content_type"];
    }

    // Cookie defaults
    if (settings.contains("cookies"))
    {
        if (settings["cookies"].contains("default_same_site"))
            cookies.default_same_site = settings["cookies"]["default_same_site"];
        if (settings["cookies"].contains("default_max_age"))
            cookies.default_max_age = settings["cookies"]["default_max_age"];
        if (settings["cookies"].contains("default_path"))
            cookies.default_path = settings["cookies"]["default_path"];
        if (settings["cookies"].contains("http_only"))
            cookies.http_only = settings["cookies"]["http_only"];
        if (settings["cookies"].contains("secure"))
            cookies.secure = settings["cookies"]["secure"];
    }

    // Logging config
    if (settings.contains("logging"))
    {
        if (settings["logging"].contains("request_logs"))
            logging.request_logs = settings["logging"]["request_logs"];
        if (settings["logging"].contains("route_registration"))
            logging.route_registration = settings["logging"]["route_registration"];
        if (settings["logging"].contains("thread_info"))
            logging.thread_info = settings["logging"]["thread_info"];
    }

    // Auto route config
    if (settings.contains("auto_routes"))
    {
        if (settings["auto_routes"].contains("enabled"))
            auto_routes.enabled = settings["auto_routes"]["enabled"];
        if (settings["auto_routes"].contains("directory"))
            auto_routes.directory = settings["auto_routes"]["directory"];
        if (settings["auto_routes"].contains("method"))
            auto_routes.method = settings["auto_routes"]["method"];
        if (settings["auto_routes"].contains("route_prefix"))
            auto_routes.route_prefix = settings["auto_routes"]["route_prefix"];
    }

    std::cout << "[INFO] Settings loaded from " << filepath << std::endl;
}
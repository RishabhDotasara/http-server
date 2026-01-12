#pragma once

#include <iostream>
#include <string>
#include <fstream>
#include <ctime>
#include <mutex>
#include <iomanip>
#include <sstream>

enum class LogLevel
{
    DEBUG = 0,
    INFO = 1,
    WARN = 2,
    ERROR = 3,
    FATAL = 4,
    NONE = 5 // Disable all logging
};

struct LoggerConfig
{
    LogLevel minLevel = LogLevel::INFO;     // Minimum level to log
    bool showTimestamp = true;              // Show timestamp
    bool showLevel = true;                  // Show log level
    bool showColors = true;                 // Use ANSI colors (terminal)
    bool logToFile = false;                 // Also log to file
    std::string logFilePath = "server.log"; // Log file path
};

class Logger
{
private:
    LoggerConfig config;
    std::ofstream fileStream;
    std::mutex logMutex;

    // ANSI color codes
    const std::string RESET = "\033[0m";
    const std::string RED = "\033[31m";
    const std::string YELLOW = "\033[33m";
    const std::string GREEN = "\033[32m";
    const std::string BLUE = "\033[34m";
    const std::string MAGENTA = "\033[35m";
    const std::string CYAN = "\033[36m";
    const std::string GRAY = "\033[90m";

    std::string getLevelString(LogLevel level)
    {
        switch (level)
        {
        case LogLevel::DEBUG:
            return "DEBUG";
        case LogLevel::INFO:
            return "INFO ";
        case LogLevel::WARN:
            return "WARN ";
        case LogLevel::ERROR:
            return "ERROR";
        case LogLevel::FATAL:
            return "FATAL";
        default:
            return "UNKNOWN";
        }
    }

    std::string getLevelColor(LogLevel level)
    {
        switch (level)
        {
        case LogLevel::DEBUG:
            return GRAY;
        case LogLevel::INFO:
            return GREEN;
        case LogLevel::WARN:
            return YELLOW;
        case LogLevel::ERROR:
            return RED;
        case LogLevel::FATAL:
            return MAGENTA;
        default:
            return RESET;
        }
    }

    std::string getTimestamp()
    {
        auto now = std::time(nullptr);
        auto tm = *std::localtime(&now);
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }

    void writeLog(LogLevel level, const std::string &message)
    {
        if (level < config.minLevel)
            return;

        std::lock_guard<std::mutex> lock(logMutex);

        std::ostringstream logLine;
        std::ostringstream coloredLine;

        // Build log line
        if (config.showTimestamp)
        {
            logLine << "[" << getTimestamp() << "] ";
            coloredLine << GRAY << "[" << getTimestamp() << "] " << RESET;
        }

        if (config.showLevel)
        {
            logLine << "[" << getLevelString(level) << "] ";
            if (config.showColors)
            {
                coloredLine << getLevelColor(level) << "[" << getLevelString(level) << "]" << RESET << " ";
            }
            else
            {
                coloredLine << "[" << getLevelString(level) << "] ";
            }
        }

        logLine << message;
        coloredLine << message;

        // Output to console
        if (config.showColors)
        {
            std::cout << coloredLine.str() << std::endl;
        }
        else
        {
            std::cout << logLine.str() << std::endl;
        }

        // Output to file
        if (config.logToFile && fileStream.is_open())
        {
            fileStream << logLine.str() << std::endl;
            fileStream.flush();
        }
    }

public:
    Logger() = default;

    Logger(LoggerConfig cfg) : config(cfg)
    {
        if (config.logToFile)
        {
            fileStream.open(config.logFilePath, std::ios::app);
        }
    }

    ~Logger()
    {
        if (fileStream.is_open())
        {
            fileStream.close();
        }
    }

    void configure(LoggerConfig cfg)
    {
        config = cfg;
        if (config.logToFile && !fileStream.is_open())
        {
            fileStream.open(config.logFilePath, std::ios::app);
        }
    }

    void setLevel(LogLevel level)
    {
        config.minLevel = level;
    }

    // Logging methods
    void debug(const std::string &message)
    {
        writeLog(LogLevel::DEBUG, message);
    }

    void info(const std::string &message)
    {
        writeLog(LogLevel::INFO, message);
    }

    void warn(const std::string &message)
    {
        writeLog(LogLevel::WARN, message);
    }

    void error(const std::string &message)
    {
        writeLog(LogLevel::ERROR, message);
    }

    void fatal(const std::string &message)
    {
        writeLog(LogLevel::FATAL, message);
    }

    // Log HTTP request
    void request(const std::string &method, const std::string &path, const std::string &status)
    {
        std::string color = CYAN;
        // Extract status code from string (e.g., "200 OK" -> 200)
        int statusCode = 200;
        try {
            statusCode = std::stoi(status);
        } catch (...) {}
        
        if (statusCode >= 400)
            color = RED;
        else if (statusCode >= 300)
            color = YELLOW;

        std::ostringstream oss;
        if (config.showColors)
        {
            oss << color << method << RESET << " " << path << " " << color << status << RESET;
        }
        else
        {
            oss << method << " " << path << " " << status;
        }

        writeLog(LogLevel::INFO, oss.str());
    }
};

// Global logger instance (optional)
extern Logger logger; 
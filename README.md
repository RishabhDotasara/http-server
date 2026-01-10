# Multithreaded Web Server

A high-performance, multithreaded HTTP/1.1 web server written in C++ with thread pooling and automatic route registration.

## Features

- ✅ **Thread Pool Architecture** - Handles multiple concurrent requests efficiently
- ✅ **Automatic Route Registration** - Auto-discovers and serves files from `public/` directory
- ✅ **HTTP/1.1 Support** - Proper HTTP headers and response handling
- ✅ **File Streaming** - Chunks large files for memory efficiency
- ✅ **Multiple Content Types** - Serves HTML, CSS, JavaScript, images, and more
- ✅ **Custom 404 Pages** - Styled error pages
- ✅ **Socket Reuse** - `SO_REUSEADDR` for immediate restarts
- ✅ **Request Parsing** - Parses HTTP methods, paths, headers, and body

## Architecture

```
Client Request → Accept Connection → Thread Pool (4 workers)
                                          ↓
                                    Parse Request
                                          ↓
                                    Route Matching
                                          ↓
                              Handler Function (pathMap)
                                          ↓
                                    Send Response
                                          ↓
                                    Close Connection
```

**Key Components:**
- **Server**: Manages socket, thread pool, and route registration
- **Request**: Parses incoming HTTP requests
- **Response**: Builds and sends HTTP responses
- **Thread Pool**: 4 worker threads with mutex + condition variable

## Quick Start

### Prerequisites

- C++17 compiler (g++, clang++)
- CMake 3.10+
- Linux/Unix system (uses POSIX sockets)

### Build

```bash
# Clone the repository
git clone https://github.com/RishabhDotasara/Multithreaded-Web-Server.git
cd Multithreaded-Web-Server

# Build with CMake
cmake -B build
cmake --build build

# Run the server
./build/server
```

### Usage

```bash
# Start the server (default: port 8081, 4 threads)
./build/server

# Access in browser
http://localhost:8081
```

The server automatically serves all files from the `public` directory:
- `/` → `public/index.html`
- `/about.html` → `public/about.html`
- `/style.css` → `public/style.css`
- etc.

## Project Structure

```
.
├── CMakeLists.txt          # Build configuration
├── main.cpp                # Entry point
├── include/                # Header files
│   ├── server.hpp
│   ├── request.hpp
│   ├── response.hpp
│   └── http.hpp
├── src/                    # Implementation files
│   ├── server.cpp
│   ├── request.cpp
│   ├── response.cpp
│   └── http.cpp
└── public/                 # Static files (auto-served)
    ├── index.html
    ├── about.html
    ├── projects.html
    ├── contact.html
    ├── 404.html
    └── style.css
```

## API Usage

### Creating Routes

```cpp
#include "server.hpp"

int main() {
    Server server(4, 8081);  // 4 threads, port 8081
    
    // Custom route handler
    server.pathMap["/api/hello"] = [](Request& req, Response& res) {
        res.sendHTML("<h1>Hello, World!</h1>");
    };
    
    // JSON response
    server.pathMap["/api/data"] = [](Request& req, Response& res) {
        std::string json = R"({"status": "ok", "message": "Hello"})";
        res.sendHTML(json);  // You can extend Response to support JSON
    };
    
    server.start();  // Blocks and serves forever
    return 0;
}
```

### Request Object

```cpp
request.data.method    // "GET", "POST", etc.
request.data.path      // "/index.html"
request.data.version   // "HTTP/1.1"
request.data.headers   // std::unordered_map<string, string>
request.data.body      // Request body content
```

### Response Object

```cpp
// Send HTML
response.sendHTML("<h1>Hello</h1>");

// Send file
std::string filepath = "public/index.html";
response.sendFile(filepath);
```

## Configuration

Edit `main.cpp` to configure:

```cpp
Server server(4, 8081);  // (number_of_threads, port)
```

Or modify `src/server.cpp`:
- Line 115: Change port number
- Line 131: Change thread pool size

## Performance

- **Concurrent Requests**: Handles up to 4 simultaneous requests (configurable)
- **File Chunking**: Streams files in 8KB chunks (memory efficient)
- **Connection Handling**: Quick accept-process-close cycle
- **Thread Safety**: Mutex-protected connection queue

### Benchmarking

```bash
# Install Apache Bench
sudo apt-get install apache2-utils

# Test with 1000 requests, 10 concurrent
ab -n 1000 -c 10 http://localhost:8081/

# Stress test
ab -n 10000 -c 100 http://localhost:8081/
```

## Supported Content Types

| Extension | MIME Type |
|-----------|-----------|
| `.html`, `.htm` | `text/html` |
| `.css` | `text/css` |
| `.js` | `application/javascript` |
| `.json` | `application/json` |
| `.png` | `image/png` |
| `.jpg`, `.jpeg` | `image/jpeg` |
| `.svg` | `image/svg+xml` |
| `.pdf` | `application/pdf` |

*(Extend in `src/response.cpp` → `getContentType()`)*

## How It Works

### 1. **Thread Pool Pattern**
- Pre-creates worker threads that wait for connections
- Uses `std::condition_variable` for efficient sleeping
- Lock-free processing after grabbing a connection

### 2. **Request Flow**
```cpp
accept() → queue.push(connfd) → cv.notify_one()
    ↓
Worker wakes up → queue.pop() → parse request
    ↓
Find handler in pathMap → execute handler
    ↓
Send response → close(connfd)
```

### 3. **Automatic Route Registration**
On startup, the server scans `public`:
```cpp
public/index.html    → pathMap["/"]
public/about.html    → pathMap["/about.html"]
public/style.css     → pathMap["/style.css"]
```

## Troubleshooting

### Port Already in Use
```bash
# Find process using port 8081
sudo lsof -i :8081

# Kill it
sudo kill -9 <PID>
```

### Permission Denied (Port < 1024)
```bash
# Use port >= 1024 or run with sudo
sudo ./build/server
```

### Files Not Loading
- Ensure files are in `public` directory
- Check file permissions: `chmod 644 public/*`
- Check server logs for 404 errors

## Development

### Adding New Routes

```cpp
// In main.cpp
server.pathMap["/custom"] = [](Request& req, Response& res) {
    res.sendHTML("<h1>Custom Page</h1>");
};
```

### Extending Response Class

```cpp
// In response.hpp
void sendJSON(const std::string& json);

// In response.cpp
void Response::sendJSON(const std::string& json) {
    std::string headers = getHTTPHeaders("200 OK", "application/json", 
                                         std::to_string(json.size()));
    send(connfd, headers.c_str(), headers.size(), 0);
    send(connfd, json.c_str(), json.size(), 0);
}
```

## Learning Resources

This project demonstrates:
- POSIX socket programming
- C++ threading (`std::thread`, `std::mutex`, `std::condition_variable`)
- HTTP protocol implementation
- File I/O and streaming
- Lambda functions and `std::function`
- Modern C++ (C++17 filesystem)

## Contributing

1. Fork the repository
2. Create a feature branch: `git checkout -b feature-name`
3. Commit changes: `git commit -am 'Add feature'`
4. Push to branch: `git push origin feature-name`
5. Submit a pull request

## License

MIT License - feel free to use this project for learning or production.

## Author

Created by [@RishabhDotasara](https://github.com/RishabhDotasara)

---

**Star ⭐ this repo if you found it helpful!**

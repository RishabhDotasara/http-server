cmake_minimum_required(VERSION 3.10)

project(HTTPServer VERSION 1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pthread")

# Debug flags
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")

# Release flags
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

# Add executable with all source files
add_executable(server 
    main.cpp
    src/server.cpp
    src/http.cpp
)

# Include directories
target_include_directories(server PRIVATE include)

# Link pthread library
target_link_libraries(server pthread)

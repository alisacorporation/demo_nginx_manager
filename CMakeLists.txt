cmake_minimum_required(VERSION 3.10)

project(nginx_manager)

# Find CURL package
find_package(CURL REQUIRED)

add_executable(nginx_manager src/main.cpp)

# Link against CURL
target_link_libraries(nginx_manager PRIVATE CURL::libcurl)
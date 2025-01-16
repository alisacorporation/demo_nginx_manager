# Nginx Server Configuration Parser

A C++ application developed as a learning project with AI assistance (Claude) to enhance C++ programming skills and modern development practices. The project demonstrates practical implementation of:

- Modern C++ features and best practices
- CMake build system configuration
- External library integration (libcurl)
- Network programming concepts
- Configuration file parsing
- System health monitoring

The application parses Nginx server configurations and performs health checks on configured domains, providing real-time status monitoring of server endpoints.

## Learning Outcomes

This project served as a hands-on learning experience for:
- Git version control
- Project structure organization
- Error handling
- Cross-platform development considerations
- Documentation practices
- Working with external libraries

## Prerequisites

- C++ compiler with C++17 support
- CMake (3.10 or higher)
- libcurl development package

### Installing Dependencies

Ubuntu/Debian:
```bash
sudo apt-get install libcurl4-openssl-dev
```

Fedora:
```bash
sudo dnf install libcurl-devel
```

macOS:
```bash
brew install curl
```

## Building the Project

```bash
mkdir build
cd build
cmake ..
make
```

## Usage
```bash
./nginx_manager /etc/nginx/vhosts
```

The program will:
1. Parse the provided Nginx configuration file
2. Detect SSL configuration for each server
3. Perform health checks on configured domains
4. Display status and health information for each server

## Development Context

Developed as part of my continuous learning journey in C++ and systems programming, with guidance and code review from AI to ensure best practices and optimal solutions.

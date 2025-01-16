#include <cstdlib>
#include <iostream>
#include <filesystem>
#include <vector>
#include <fstream>
#include <algorithm>
#include <curl/curl.h>

// make struct for vhost config and name it server
struct Server {
    short int id;
    std::string filename;
    std::string name;
    std::string root;
    std::string index;
    bool ssl;
    std::string ssl_cert;
    std::string ssl_key;
    bool status;
};

struct ServerHealth {
    bool is_healthy;
    long status_code;
};

// Add callback function for CURL
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    return size * nmemb;  // Just return the size of the received data
}

// Modify the function to return both health status and code
ServerHealth checkServerHealth(const std::string& server_name, bool ssl) {
    CURL* curl = curl_easy_init();
    ServerHealth result = {false, 0};
    
    if (curl) {
        std::string url = (ssl ? "https://" : "http://") + server_name;
        
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        
        CURLcode res = curl_easy_perform(curl);
        
        if (res == CURLE_OK) {
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &result.status_code);
            result.is_healthy = (result.status_code >= 200 && result.status_code < 400);
        }
        
        curl_easy_cleanup(curl);
    }
    
    return result;
}

int main(int argc, char* argv[]) {
    // Initialize CURL
    curl_global_init(CURL_GLOBAL_DEFAULT);

    // init array of server structs
    std::vector<Server> servers;

    // if argument not provided, show error & usage
    if (argc != 2) {
        std::cerr << "Usage: " << "./nginx_manager" << " <vhost_directory_path>" << std::endl;
        return EXIT_FAILURE;
    }

    // fetch vhost directory from command line argument
    std::string vhost_dir = argv[1];

    // check if the directory exists
    if (!std::filesystem::exists(vhost_dir)) {
        std::cerr << "Directory " << vhost_dir << " does not exist" << std::endl;
        return EXIT_FAILURE;
    }

    // read all config files from /etc/nginx/vhosts
    std::vector<std::string> config_files;
    for (const auto& entry : std::filesystem::directory_iterator(vhost_dir)) {
        if (entry.path().extension() == ".conf") {
            config_files.push_back(entry.path().string());
        }
    }

    // sort config files by name
    std::sort(config_files.begin(), config_files.end());

    // loop each config file
    for (const auto& config_file : config_files) {
        // read config file and fill server struct
        Server server;
        server.filename = config_file;

        // server id is the index of the config file in the config_files array, incremented by 1
        server.id = std::distance(config_files.begin(), std::find(config_files.begin(), config_files.end(), config_file)) + 1;

        // server name is the name of the config file without the extension and without the /etc/nginx/vhost/ prefix    
        server.name = config_file.substr(config_file.find_last_of('/') + 1, config_file.find_last_of('.') - config_file.find_last_of('/') - 1);

        // get server directory from config file
        std::ifstream file(config_file);
        std::string line;

        // get server root directory from config file
        while (std::getline(file, line)) {
            if (line.find("root") != std::string::npos && line.find("SCRIPT_FILENAME") == std::string::npos) {
                server.root = line.substr(line.find("root") + 5, line.find(";") - line.find("root") - 5);
            }
        }

        // get server index from config file
        line = "";
        while (std::getline(file, line)) {
            if (line.find("index") != std::string::npos) {
                server.index = line.substr(line.find("index") + 6, line.find(";") - line.find("index") - 6);
            }
        }

        // get server ssl from config file
        file.clear();                 // Clear any error flags
        file.seekg(0, std::ios::beg); // Reset file pointer to beginning
        
        server.ssl = false;  // Default to false
        line = "";
        while (std::getline(file, line)) {
            // Remove whitespace and check for "listen" with "ssl"
            std::string trimmed = line;
            trimmed.erase(0, trimmed.find_first_not_of(" \t"));  // Left trim
            trimmed.erase(trimmed.find_last_not_of(" \t") + 1);  // Right trim
            
            if (trimmed.find("listen") != std::string::npos && 
                trimmed.find("ssl") != std::string::npos) {
                server.ssl = true;
                break;
            }
        }

        // get server ssl cert from config file
        line = "";
        while (std::getline(file, line)) {
            if (line.find("ssl_certificate") != std::string::npos && line.find("ssl_certificate_key") == std::string::npos) {
                server.ssl_cert = line.substr(line.find("ssl_certificate") + 17, line.find(";") - line.find("ssl_certificate") - 17);
            }
        }

        // get server ssl key from config file
        line = "";
        while (std::getline(file, line)) {
            if (line.find("ssl_certificate_key") != std::string::npos) {
                server.ssl_key = line.substr(line.find("ssl_certificate_key") + 21, line.find(";") - line.find("ssl_certificate_key") - 21);
            }
        }

        // Check server health status
        ServerHealth health = checkServerHealth(server.name, server.ssl);
        server.status = health.is_healthy;

        // add server to servers array
        servers.push_back(server);
    }

    // print all servers
    for (const auto& server : servers) {
        ServerHealth health = checkServerHealth(server.name, server.ssl);

        std::cout << "Server ID: " << server.id 
                 << ", Name: " << server.name 
                 << ", Root: " << server.root 
                 << ", Index: " << server.index 
                 << ", SSL: " << (server.ssl ? "enabled" : "disabled")
                 << ", Status: " << (server.status ? "healthy" : "unhealthy") 
                 << " (" << health.status_code << ")" << std::endl;
    }

    // Cleanup CURL
    curl_global_cleanup();

    return EXIT_SUCCESS;
}
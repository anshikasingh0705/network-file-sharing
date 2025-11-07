// server.cpp - File Sharing Server (Day 2: File Listing)
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <sys/stat.h>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>

#define PORT 8080
#define BUFFER_SIZE 4096
#define SHARED_DIR "./shared_files"

struct FileInfo {
    std::string name;
    long size;
    bool is_directory;
    std::string permissions;
};

class FileServer {
private:
    int server_fd;
    int client_socket;
    struct sockaddr_in address;
    int addrlen;

    // Get file size in bytes
    long getFileSize(const std::string& filepath) {
        struct stat st;
        if (stat(filepath.c_str(), &st) == 0) {
            return st.st_size;
        }
        return 0;
    }

    // Get human-readable file size
    std::string formatFileSize(long bytes) {
        const char* units[] = {"B", "KB", "MB", "GB"};
        int unit = 0;
        double size = bytes;
        
        while (size >= 1024 && unit < 3) {
            size /= 1024;
            unit++;
        }
        
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2) << size << " " << units[unit];
        return oss.str();
    }

    // Get permissions string
    std::string getPermissions(const std::string& filepath) {
        struct stat st;
        if (stat(filepath.c_str(), &st) != 0) {
            return "?????????";
        }
        
        std::string perms = "";
        perms += (S_ISDIR(st.st_mode)) ? 'd' : '-';
        perms += (st.st_mode & S_IRUSR) ? 'r' : '-';
        perms += (st.st_mode & S_IWUSR) ? 'w' : '-';
        perms += (st.st_mode & S_IXUSR) ? 'x' : '-';
        perms += (st.st_mode & S_IRGRP) ? 'r' : '-';
        perms += (st.st_mode & S_IWGRP) ? 'w' : '-';
        perms += (st.st_mode & S_IXGRP) ? 'x' : '-';
        perms += (st.st_mode & S_IROTH) ? 'r' : '-';
        perms += (st.st_mode & S_IWOTH) ? 'w' : '-';
        perms += (st.st_mode & S_IXOTH) ? 'x' : '-';
        
        return perms;
    }

    // List files in shared directory
    std::vector<FileInfo> listFiles() {
        std::vector<FileInfo> files;
        DIR* dir = opendir(SHARED_DIR);
        
        if (!dir) {
            std::cerr << "Error: Cannot open shared directory" << std::endl;
            return files;
        }
        
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            // Skip . and ..
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }
            
            FileInfo info;
            info.name = entry->d_name;
            
            std::string fullpath = std::string(SHARED_DIR) + "/" + entry->d_name;
            info.size = getFileSize(fullpath);
            info.is_directory = (entry->d_type == DT_DIR);
            info.permissions = getPermissions(fullpath);
            
            files.push_back(info);
        }
        
        closedir(dir);
        return files;
    }

    // Handle LIST command
    void handleList() {
        std::vector<FileInfo> files = listFiles();
        
        if (files.empty()) {
            sendMessage("ERROR: No files in shared directory\n");
            return;
        }
        
        std::ostringstream response;
        response << "OK\n";
        response << "Files in shared directory:\n";
        response << std::string(70, '-') << "\n";
        response << std::left << std::setw(30) << "Name" 
                 << std::setw(15) << "Size" 
                 << std::setw(12) << "Type"
                 << "Permissions\n";
        response << std::string(70, '-') << "\n";
        
        for (const auto& file : files) {
            response << std::left << std::setw(30) << file.name
                     << std::setw(15) << formatFileSize(file.size)
                     << std::setw(12) << (file.is_directory ? "[DIR]" : "[FILE]")
                     << file.permissions << "\n";
        }
        
        response << std::string(70, '-') << "\n";
        response << "Total: " << files.size() << " items\n";
        
        sendMessage(response.str());
    }

    // Handle INFO command
    void handleInfo(const std::string& filename) {
        if (filename.empty()) {
            sendMessage("ERROR: Filename required\n");
            return;
        }
        
        std::string filepath = std::string(SHARED_DIR) + "/" + filename;
        struct stat st;
        
        if (stat(filepath.c_str(), &st) != 0) {
            sendMessage("ERROR: File not found\n");
            return;
        }
        
        std::ostringstream response;
        response << "OK\n";
        response << "File Information:\n";
        response << std::string(40, '-') << "\n";
        response << "Name:        " << filename << "\n";
        response << "Size:        " << formatFileSize(st.st_size) << " (" << st.st_size << " bytes)\n";
        response << "Type:        " << (S_ISDIR(st.st_mode) ? "Directory" : "Regular File") << "\n";
        response << "Permissions: " << getPermissions(filepath) << "\n";
        response << std::string(40, '-') << "\n";
        
        sendMessage(response.str());
    }

    // Send message to client
    void sendMessage(const std::string& message) {
        send(client_socket, message.c_str(), message.length(), 0);
    }

    // Process command from client
    void processCommand(const std::string& command) {
        std::istringstream iss(command);
        std::string cmd;
        iss >> cmd;
        
        std::cout << "Processing command: " << cmd << std::endl;
        
        if (cmd == "LIST") {
            handleList();
        } 
        else if (cmd == "INFO") {
            std::string filename;
            iss >> filename;
            handleInfo(filename);
        }
        else if (cmd == "HELP") {
            std::string help = 
                "Available Commands:\n"
                "  LIST           - List all files in shared directory\n"
                "  INFO <file>    - Get detailed info about a file\n"
                "  HELP           - Show this help message\n"
                "  EXIT           - Disconnect from server\n";
            sendMessage(help);
        }
        else if (cmd == "EXIT") {
            sendMessage("Goodbye!\n");
        }
        else {
            sendMessage("ERROR: Unknown command. Type HELP for available commands.\n");
        }
    }

public:
    FileServer() : server_fd(0), client_socket(0), addrlen(sizeof(address)) {
        address = {};
    }

    bool initialize() {
        // Create shared directory if it doesn't exist
        struct stat st = {0};
        if (stat(SHARED_DIR, &st) == -1) {
            if (mkdir(SHARED_DIR, 0755) == 0) {
                std::cout << "✓ Created shared directory: " << SHARED_DIR << std::endl;
            } else {
                std::cerr << "✗ Failed to create shared directory" << std::endl;
            }
        }

        // Create socket
        if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
            perror("Socket creation failed");
            return false;
        }

        // Set socket options
        int opt = 1;
        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                       &opt, sizeof(opt))) {
            perror("Setsockopt failed");
            return false;
        }

        // Configure address
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(PORT);

        // Bind
        if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
            perror("Bind failed");
            return false;
        }

        // Listen
        if (listen(server_fd, 3) < 0) {
            perror("Listen failed");
            return false;
        }

        std::cout << "✓ Server initialized successfully" << std::endl;
        std::cout << "✓ Listening on port " << PORT << std::endl;
        std::cout << "✓ Shared directory: " << SHARED_DIR << std::endl;
        return true;
    }

    void acceptConnection() {
        std::cout << "\nWaiting for client connection..." << std::endl;
        
        if ((client_socket = accept(server_fd, (struct sockaddr *)&address,
                                   (socklen_t*)&addrlen)) < 0) {
            perror("Accept failed");
            return;
        }

        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(address.sin_addr), client_ip, INET_ADDRSTRLEN);
        
        std::cout << "✓ Client connected from " << client_ip 
                  << ":" << ntohs(address.sin_port) << std::endl;
    }

    void handleClient() {
        char buffer[BUFFER_SIZE] = {0};
        
        // Send welcome message
        std::string welcome = 
            "=== Welcome to File Sharing Server ===\n"
            "Type HELP to see available commands\n\n";
        sendMessage(welcome);

        while (true) {
            memset(buffer, 0, BUFFER_SIZE);
            
            int bytes_read = read(client_socket, buffer, BUFFER_SIZE);
            
            if (bytes_read <= 0) {
                std::cout << "✗ Client disconnected" << std::endl;
                break;
            }

            std::string command(buffer);
            // Remove trailing newline
            if (!command.empty() && command.back() == '\n') {
                command.pop_back();
            }

            std::cout << "Received: " << command << std::endl;

            if (command == "EXIT") {
                sendMessage("Goodbye!\n");
                std::cout << "Client requested disconnection" << std::endl;
                break;
            }

            processCommand(command);
        }

        close(client_socket);
    }

    void run() {
        if (!initialize()) {
            return;
        }

        while (true) {
            acceptConnection();
            handleClient();
        }
    }

    ~FileServer() {
        if (server_fd > 0) {
            close(server_fd);
        }
        std::cout << "Server shutdown complete" << std::endl;
    }
};

int main() {
    std::cout << "=== File Sharing Server (Day 2) ===" << std::endl;
    
    FileServer server;
    server.run();

    return 0;
}

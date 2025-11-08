// server.cpp - File Sharing Server (Day 5: Authentication & Security)
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
#include <fstream>
#include <map>
#include <ctime>

#define PORT 8080
#define BUFFER_SIZE 4096
#define SHARED_DIR "./shared_files"
#define CHUNK_SIZE 4096
#define LOG_FILE "./server.log"
#define USERS_FILE "./users.txt"

struct FileInfo {
    std::string name;
    long size;
    bool is_directory;
    std::string permissions;
};

struct User {
    std::string username;
    std::string password;
    bool can_upload;
    bool can_download;
};

class FileServer {
private:
    int server_fd;
    int client_socket;
    struct sockaddr_in address;
    int addrlen;
    std::map<std::string, User> users;
    bool is_authenticated;
    std::string current_user;
    std::string client_ip;

    void logActivity(const std::string& activity) {
        std::ofstream logfile(LOG_FILE, std::ios::app);
        if (logfile.is_open()) {
            time_t now = time(0);
            char* dt = ctime(&now);
            std::string timestamp(dt);
            timestamp.pop_back(); // Remove newline
            
            logfile << "[" << timestamp << "] "
                   << "[" << client_ip << "] "
                   << "[" << (is_authenticated ? current_user : "ANONYMOUS") << "] "
                   << activity << std::endl;
            logfile.close();
        }
    }

    void loadUsers() {
        std::ifstream userfile(USERS_FILE);
        if (!userfile.is_open()) {
            // Create default users file
            std::ofstream outfile(USERS_FILE);
            if (outfile.is_open()) {
                outfile << "admin:admin123:1:1\n";
                outfile << "user:user123:0:1\n";
                outfile << "uploader:upload123:1:0\n";
                outfile.close();
                std::cout << "✓ Created default users file" << std::endl;
                std::cout << "  Default users: admin/admin123, user/user123, uploader/upload123" << std::endl;
            }
            userfile.open(USERS_FILE);
        }

        if (userfile.is_open()) {
            std::string line;
            while (std::getline(userfile, line)) {
                std::istringstream iss(line);
                std::string username, password, upload, download;
                
                if (std::getline(iss, username, ':') &&
                    std::getline(iss, password, ':') &&
                    std::getline(iss, upload, ':') &&
                    std::getline(iss, download)) {
                    
                    User user;
                    user.username = username;
                    user.password = password;
                    user.can_upload = (upload == "1");
                    user.can_download = (download == "1");
                    
                    users[username] = user;
                }
            }
            userfile.close();
            std::cout << "✓ Loaded " << users.size() << " users" << std::endl;
        }
    }

    bool authenticateUser(const std::string& username, const std::string& password) {
        auto it = users.find(username);
        if (it != users.end() && it->second.password == password) {
            current_user = username;
            is_authenticated = true;
            logActivity("LOGIN SUCCESS");
            return true;
        }
        logActivity("LOGIN FAILED - User: " + username);
        return false;
    }

    long getFileSize(const std::string& filepath) {
        struct stat st;
        if (stat(filepath.c_str(), &st) == 0) {
            return st.st_size;
        }
        return 0;
    }

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

    std::vector<FileInfo> listFiles() {
        std::vector<FileInfo> files;
        DIR* dir = opendir(SHARED_DIR);
        
        if (!dir) {
            return files;
        }
        
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
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

    void handleLogin(const std::string& credentials) {
        std::istringstream iss(credentials);
        std::string username, password;
        
        if (std::getline(iss, username, ':') && std::getline(iss, password)) {
            if (authenticateUser(username, password)) {
                std::ostringstream response;
                response << "OK\n";
                response << "Login successful! Welcome, " << current_user << "\n";
                response << "Permissions:\n";
                response << "  - Upload: " << (users[current_user].can_upload ? "YES" : "NO") << "\n";
                response << "  - Download: " << (users[current_user].can_download ? "YES" : "NO") << "\n";
                sendMessage(response.str());
                std::cout << "✓ User authenticated: " << current_user << std::endl;
            } else {
                sendMessage("ERROR: Invalid username or password\n");
                std::cout << "✗ Authentication failed" << std::endl;
            }
        } else {
            sendMessage("ERROR: Invalid login format\n");
        }
    }

    void handleList() {
        if (!is_authenticated) {
            sendMessage("ERROR: Authentication required\n");
            logActivity("UNAUTHORIZED ACCESS - LIST");
            return;
        }

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
        logActivity("LIST - " + std::to_string(files.size()) + " items");
    }

    void handleInfo(const std::string& filename) {
        if (!is_authenticated) {
            sendMessage("ERROR: Authentication required\n");
            logActivity("UNAUTHORIZED ACCESS - INFO");
            return;
        }

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
        logActivity("INFO - " + filename);
    }

    void handleDownload(const std::string& filename) {
        if (!is_authenticated) {
            sendMessage("ERROR: Authentication required\n");
            logActivity("UNAUTHORIZED ACCESS - DOWNLOAD");
            return;
        }

        if (!users[current_user].can_download) {
            sendMessage("ERROR: Permission denied - You cannot download files\n");
            logActivity("PERMISSION DENIED - DOWNLOAD - " + filename);
            return;
        }
        
        if (filename.empty()) {
            sendMessage("ERROR: Filename required\n");
            return;
        }
        
        std::string filepath = std::string(SHARED_DIR) + "/" + filename;
        
        std::ifstream file(filepath, std::ios::binary);
        if (!file.is_open()) {
            sendMessage("ERROR: File not found or cannot be opened\n");
            return;
        }
        
        file.seekg(0, std::ios::end);
        long filesize = file.tellg();
        file.seekg(0, std::ios::beg);
        
        struct stat st;
        if (stat(filepath.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) {
            sendMessage("ERROR: Cannot download directories\n");
            file.close();
            return;
        }
        
        std::cout << "📤 " << current_user << " downloading: " << filename 
                  << " (" << formatFileSize(filesize) << ")" << std::endl;
        
        std::ostringstream metadata;
        metadata << "OK\n";
        metadata << "FILESIZE:" << filesize << "\n";
        metadata << "FILENAME:" << filename << "\n";
        metadata << "START\n";
        sendMessage(metadata.str());
        
        char ack[10] = {0};
        read(client_socket, ack, sizeof(ack));
        
        if (strncmp(ack, "READY", 5) != 0) {
            file.close();
            return;
        }
        
        char buffer[CHUNK_SIZE];
        long bytes_sent = 0;
        
        while (!file.eof() && bytes_sent < filesize) {
            file.read(buffer, CHUNK_SIZE);
            std::streamsize bytes_read = file.gcount();
            
            if (bytes_read > 0) {
                ssize_t sent = send(client_socket, buffer, bytes_read, 0);
                if (sent < 0) break;
                bytes_sent += sent;
            }
        }
        
        file.close();
        std::cout << "✓ Download complete: " << filename << std::endl;
        logActivity("DOWNLOAD - " + filename + " (" + std::to_string(bytes_sent) + " bytes)");
    }

    void handleUpload(const std::string& filename) {
        if (!is_authenticated) {
            sendMessage("ERROR: Authentication required\n");
            logActivity("UNAUTHORIZED ACCESS - UPLOAD");
            return;
        }

        if (!users[current_user].can_upload) {
            sendMessage("ERROR: Permission denied - You cannot upload files\n");
            logActivity("PERMISSION DENIED - UPLOAD - " + filename);
            return;
        }
        
        if (filename.empty()) {
            sendMessage("ERROR: Filename required\n");
            return;
        }
        
        sendMessage("READY\n");
        
        char buffer[BUFFER_SIZE] = {0};
        int bytes_read = read(client_socket, buffer, BUFFER_SIZE);
        
        if (bytes_read <= 0) {
            return;
        }
        
        std::string metadata(buffer);
        std::istringstream iss(metadata);
        std::string line;
        long filesize = 0;
        std::string recv_filename;
        bool start_found = false;
        
        while (std::getline(iss, line)) {
            if (line.find("FILESIZE:") != std::string::npos) {
                filesize = std::stol(line.substr(9));
            } else if (line.find("FILENAME:") != std::string::npos) {
                recv_filename = line.substr(9);
            } else if (line.find("START") != std::string::npos) {
                start_found = true;
                break;
            }
        }
        
        if (!start_found || filesize == 0) {
            sendMessage("ERROR: Invalid metadata\n");
            return;
        }
        
        std::cout << "📥 " << current_user << " uploading: " << recv_filename 
                  << " (" << formatFileSize(filesize) << ")" << std::endl;
        
        std::string filepath = std::string(SHARED_DIR) + "/" + recv_filename;
        std::ofstream outfile(filepath, std::ios::binary);
        
        if (!outfile.is_open()) {
            sendMessage("ERROR: Cannot create file\n");
            return;
        }
        
        send(client_socket, "READY", 5, 0);
        
        long bytes_received = 0;
        char data_buffer[CHUNK_SIZE];
        
        while (bytes_received < filesize) {
            memset(data_buffer, 0, CHUNK_SIZE);
            
            long remaining = filesize - bytes_received;
            int to_read = (remaining < CHUNK_SIZE) ? remaining : CHUNK_SIZE;
            
            int received = read(client_socket, data_buffer, to_read);
            
            if (received <= 0) {
                outfile.close();
                sendMessage("ERROR: Upload failed\n");
                return;
            }
            
            outfile.write(data_buffer, received);
            bytes_received += received;
        }
        
        outfile.close();
        
        std::cout << "✓ Upload complete: " << recv_filename << std::endl;
        logActivity("UPLOAD - " + recv_filename + " (" + std::to_string(bytes_received) + " bytes)");
        
        sendMessage("OK: Upload successful\n");
    }

    void sendMessage(const std::string& message) {
        send(client_socket, message.c_str(), message.length(), 0);
    }

    void processCommand(const std::string& command) {
        std::istringstream iss(command);
        std::string cmd;
        iss >> cmd;
        
        std::cout << "Processing command: " << cmd;
        if (is_authenticated) {
            std::cout << " [User: " << current_user << "]";
        }
        std::cout << std::endl;
        
        if (cmd == "LOGIN") {
            std::string credentials;
            std::getline(iss, credentials);
            credentials = credentials.substr(1); // Remove leading space
            handleLogin(credentials);
        }
        else if (cmd == "LIST") {
            handleList();
        } 
        else if (cmd == "INFO") {
            std::string filename;
            iss >> filename;
            handleInfo(filename);
        }
        else if (cmd == "DOWNLOAD") {
            std::string filename;
            iss >> filename;
            handleDownload(filename);
        }
        else if (cmd == "UPLOAD") {
            std::string filename;
            iss >> filename;
            handleUpload(filename);
        }
        else if (cmd == "LOGOUT") {
            if (is_authenticated) {
                logActivity("LOGOUT");
                std::cout << "✓ User logged out: " << current_user << std::endl;
                current_user = "";
                is_authenticated = false;
                sendMessage("OK: Logged out successfully\n");
            } else {
                sendMessage("ERROR: Not logged in\n");
            }
        }
        else if (cmd == "HELP") {
            std::string help;
            if (!is_authenticated) {
                help = "Available Commands:\n"
                       "  LOGIN <user>:<pass> - Authenticate with server\n"
                       "  HELP                - Show this help message\n"
                       "  EXIT                - Disconnect from server\n";
            } else {
                help = "Available Commands:\n"
                       "  LIST                - List all files\n"
                       "  INFO <file>         - Get file information\n"
                       "  DOWNLOAD <file>     - Download a file\n"
                       "  UPLOAD <file>       - Upload a file\n"
                       "  LOGOUT              - Logout from server\n"
                       "  HELP                - Show this help\n"
                       "  EXIT                - Disconnect\n";
            }
            sendMessage(help);
        }
        else if (cmd == "EXIT") {
            if (is_authenticated) {
                logActivity("DISCONNECT");
            }
            sendMessage("Goodbye!\n");
        }
        else {
            sendMessage("ERROR: Unknown command. Type HELP for available commands.\n");
        }
    }

public:
    FileServer() : server_fd(0), client_socket(0), addrlen(sizeof(address)), 
                   is_authenticated(false), current_user(""), client_ip("") {
        address = {};
    }

    bool initialize() {
        loadUsers();
        
        struct stat st = {0};
        if (stat(SHARED_DIR, &st) == -1) {
            if (mkdir(SHARED_DIR, 0755) == 0) {
                std::cout << "✓ Created shared directory: " << SHARED_DIR << std::endl;
            }
        }

        if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
            perror("Socket creation failed");
            return false;
        }

        int opt = 1;
        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                       &opt, sizeof(opt))) {
            perror("Setsockopt failed");
            return false;
        }

        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(PORT);

        if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
            perror("Bind failed");
            return false;
        }

        if (listen(server_fd, 3) < 0) {
            perror("Listen failed");
            return false;
        }

        std::cout << "✓ Server initialized successfully" << std::endl;
        std::cout << "✓ Listening on port " << PORT << std::endl;
        std::cout << "✓ Shared directory: " << SHARED_DIR << std::endl;
        std::cout << "✓ Logging to: " << LOG_FILE << std::endl;
        return true;
    }

    void acceptConnection() {
        std::cout << "\nWaiting for client connection..." << std::endl;
        
        if ((client_socket = accept(server_fd, (struct sockaddr *)&address,
                                   (socklen_t*)&addrlen)) < 0) {
            perror("Accept failed");
            return;
        }

        char ip_buffer[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(address.sin_addr), ip_buffer, INET_ADDRSTRLEN);
        client_ip = std::string(ip_buffer);
        
        std::cout << "✓ Client connected from " << client_ip 
                  << ":" << ntohs(address.sin_port) << std::endl;
        
        is_authenticated = false;
        current_user = "";
        logActivity("CONNECTED");
    }

    void handleClient() {
        char buffer[BUFFER_SIZE] = {0};
        
        std::string welcome = 
            "=== Secure File Sharing Server ===\n"
            "Please login to continue.\n"
            "Type HELP to see available commands\n\n";
        sendMessage(welcome);

        while (true) {
            memset(buffer, 0, BUFFER_SIZE);
            
            int bytes_read = read(client_socket, buffer, BUFFER_SIZE);
            
            if (bytes_read <= 0) {
                std::cout << "✗ Client disconnected" << std::endl;
                if (is_authenticated) {
                    logActivity("DISCONNECTED");
                }
                break;
            }

            std::string command(buffer);
            if (!command.empty() && command.back() == '\n') {
                command.pop_back();
            }

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
    std::cout << "=== Secure File Sharing Server (Day 5) ===" << std::endl;
    
    FileServer server;
    server.run();

    return 0;
}

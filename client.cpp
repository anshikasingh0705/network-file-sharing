// client.cpp - File Sharing Client (Day 4: File Upload)
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <sys/stat.h>
#include <dirent.h>

#define PORT 8080
#define BUFFER_SIZE 4096
#define DOWNLOAD_DIR "./downloads"
#define UPLOAD_DIR "./uploads"

class FileClient {
private:
    int sock;
    struct sockaddr_in serv_addr;
    bool connected;

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

    void sendCommand(const std::string& command) {
        send(sock, command.c_str(), command.length(), 0);
    }

    std::string receiveResponse() {
        char buffer[BUFFER_SIZE] = {0};
        int bytes_read = read(sock, buffer, BUFFER_SIZE);
        
        if (bytes_read <= 0) {
            std::cout << "✗ Server disconnected" << std::endl;
            connected = false;
            return "";
        }
        
        return std::string(buffer);
    }

    void displayMenu() {
        std::cout << "\n╔════════════════════════════════════════╗" << std::endl;
        std::cout << "║    File Sharing Client - Menu          ║" << std::endl;
        std::cout << "╠════════════════════════════════════════╣" << std::endl;
        std::cout << "║  1. LIST     - List server files       ║" << std::endl;
        std::cout << "║  2. INFO     - Get file information    ║" << std::endl;
        std::cout << "║  3. DOWNLOAD - Download from server    ║" << std::endl;
        std::cout << "║  4. UPLOAD   - Upload to server        ║" << std::endl;
        std::cout << "║  5. HELP     - Show server commands    ║" << std::endl;
        std::cout << "║  6. EXIT     - Disconnect              ║" << std::endl;
        std::cout << "╚════════════════════════════════════════╝" << std::endl;
        std::cout << "\nEnter command or number: ";
    }

    void handleListCommand() {
        std::cout << "\n📁 Requesting file list from server...\n" << std::endl;
        sendCommand("LIST\n");
        
        std::string response = receiveResponse();
        if (!response.empty()) {
            std::cout << response << std::endl;
        }
    }

    void handleInfoCommand() {
        std::cout << "\nEnter filename: ";
        std::string filename;
        std::getline(std::cin, filename);
        
        if (filename.empty()) {
            std::cout << "Error: Filename cannot be empty" << std::endl;
            return;
        }
        
        std::string command = "INFO " + filename + "\n";
        sendCommand(command);
        
        std::string response = receiveResponse();
        if (!response.empty()) {
            std::cout << "\n" << response << std::endl;
        }
    }

    void handleDownloadCommand() {
        std::cout << "\nEnter filename to download: ";
        std::string filename;
        std::getline(std::cin, filename);
        
        if (filename.empty()) {
            std::cout << "Error: Filename cannot be empty" << std::endl;
            return;
        }
        
        system(("mkdir -p " + std::string(DOWNLOAD_DIR)).c_str());
        
        std::cout << "\n📥 Requesting download: " << filename << std::endl;
        
        std::string command = "DOWNLOAD " + filename + "\n";
        sendCommand(command);
        
        char buffer[BUFFER_SIZE] = {0};
        int bytes_read = read(sock, buffer, BUFFER_SIZE);
        
        if (bytes_read <= 0) {
            std::cout << "✗ Server disconnected" << std::endl;
            connected = false;
            return;
        }
        
        std::string response(buffer);
        
        if (response.find("ERROR") != std::string::npos) {
            std::cout << response << std::endl;
            return;
        }
        
        std::istringstream iss(response);
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
            std::cout << "Error: Invalid file metadata received" << std::endl;
            return;
        }
        
        std::cout << "File size: " << formatFileSize(filesize) 
                  << " (" << filesize << " bytes)" << std::endl;
        std::cout << "Saving to: " << DOWNLOAD_DIR << "/" << recv_filename << std::endl;
        
        send(sock, "READY", 5, 0);
        
        std::string filepath = std::string(DOWNLOAD_DIR) + "/" + recv_filename;
        std::ofstream outfile(filepath, std::ios::binary);
        
        if (!outfile.is_open()) {
            std::cout << "Error: Cannot create file for writing" << std::endl;
            return;
        }
        
        long bytes_received = 0;
        char data_buffer[BUFFER_SIZE];
        
        std::cout << "\n🔄 Downloading..." << std::endl;
        std::cout << "Progress: [" << std::flush;
        
        int last_progress = -1;
        
        while (bytes_received < filesize) {
            memset(data_buffer, 0, BUFFER_SIZE);
            
            long remaining = filesize - bytes_received;
            int to_read = (remaining < BUFFER_SIZE) ? remaining : BUFFER_SIZE;
            
            int received = read(sock, data_buffer, to_read);
            
            if (received <= 0) {
                std::cout << "\n✗ Error receiving file data" << std::endl;
                outfile.close();
                return;
            }
            
            outfile.write(data_buffer, received);
            bytes_received += received;
            
            int progress = (bytes_received * 50) / filesize;
            if (progress != last_progress) {
                for (int i = last_progress + 1; i <= progress; i++) {
                    std::cout << "=" << std::flush;
                }
                last_progress = progress;
            }
        }
        
        std::cout << "] 100%" << std::endl;
        outfile.close();
        
        std::cout << "\n✓ Download complete!" << std::endl;
        std::cout << "  File saved: " << filepath << std::endl;
        std::cout << "  Size: " << formatFileSize(bytes_received) 
                  << " (" << bytes_received << " bytes)" << std::endl;
    }

    void listLocalFiles() {
        std::cout << "\n📂 Files available for upload:\n" << std::endl;
        
        DIR* dir = opendir(UPLOAD_DIR);
        if (!dir) {
            std::cout << "No files found in " << UPLOAD_DIR << std::endl;
            std::cout << "Create the directory and add files to upload." << std::endl;
            return;
        }
        
        std::cout << std::string(60, '-') << std::endl;
        std::cout << std::left << std::setw(35) << "Filename" << "Size" << std::endl;
        std::cout << std::string(60, '-') << std::endl;
        
        int count = 0;
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }
            
            if (entry->d_type == DT_DIR) {
                continue; // Skip directories
            }
            
            std::string filepath = std::string(UPLOAD_DIR) + "/" + entry->d_name;
            long size = getFileSize(filepath);
            
            std::cout << std::left << std::setw(35) << entry->d_name 
                     << formatFileSize(size) << std::endl;
            count++;
        }
        
        closedir(dir);
        std::cout << std::string(60, '-') << std::endl;
        std::cout << "Total: " << count << " file(s)" << std::endl;
    }

    void handleUploadCommand() {
        // Create upload directory if doesn't exist
        system(("mkdir -p " + std::string(UPLOAD_DIR)).c_str());
        
        // Show available files
        listLocalFiles();
        
        std::cout << "\nEnter filename to upload (from " << UPLOAD_DIR << "): ";
        std::string filename;
        std::getline(std::cin, filename);
        
        if (filename.empty()) {
            std::cout << "Error: Filename cannot be empty" << std::endl;
            return;
        }
        
        std::string filepath = std::string(UPLOAD_DIR) + "/" + filename;
        
        // Check if file exists
        std::ifstream file(filepath, std::ios::binary);
        if (!file.is_open()) {
            std::cout << "Error: File not found: " << filepath << std::endl;
            std::cout << "Make sure the file is in the " << UPLOAD_DIR << " directory" << std::endl;
            return;
        }
        
        // Get file size
        file.seekg(0, std::ios::end);
        long filesize = file.tellg();
        file.seekg(0, std::ios::beg);
        
        std::cout << "\n📤 Uploading: " << filename 
                  << " (" << formatFileSize(filesize) << ")" << std::endl;
        
        // Send upload command
        std::string command = "UPLOAD " + filename + "\n";
        sendCommand(command);
        
        // Wait for server ready signal
        char buffer[BUFFER_SIZE] = {0};
        int bytes_read = read(sock, buffer, BUFFER_SIZE);
        
        if (bytes_read <= 0 || strncmp(buffer, "READY", 5) != 0) {
            std::cout << "Error: Server not ready" << std::endl;
            file.close();
            return;
        }
        
        // Send file metadata
        std::ostringstream metadata;
        metadata << "FILESIZE:" << filesize << "\n";
        metadata << "FILENAME:" << filename << "\n";
        metadata << "START\n";
        sendCommand(metadata.str());
        
        // Wait for acknowledgment
        memset(buffer, 0, BUFFER_SIZE);
        bytes_read = read(sock, buffer, BUFFER_SIZE);
        
        if (bytes_read <= 0 || strncmp(buffer, "READY", 5) != 0) {
            std::cout << "Error: Server not ready to receive file" << std::endl;
            file.close();
            return;
        }
        
        // Send file data
        char data_buffer[BUFFER_SIZE];
        long bytes_sent = 0;
        int chunk_count = 0;
        
        std::cout << "\n🔄 Uploading..." << std::endl;
        std::cout << "Progress: [" << std::flush;
        
        int last_progress = -1;
        
        while (!file.eof() && bytes_sent < filesize) {
            file.read(data_buffer, BUFFER_SIZE);
            std::streamsize bytes_read_chunk = file.gcount();
            
            if (bytes_read_chunk > 0) {
                ssize_t sent = send(sock, data_buffer, bytes_read_chunk, 0);
                if (sent < 0) {
                    std::cout << "\n✗ Error sending file data" << std::endl;
                    file.close();
                    return;
                }
                bytes_sent += sent;
                chunk_count++;
                
                int progress = (bytes_sent * 50) / filesize;
                if (progress != last_progress) {
                    for (int i = last_progress + 1; i <= progress; i++) {
                        std::cout << "=" << std::flush;
                    }
                    last_progress = progress;
                }
            }
        }
        
        std::cout << "] 100%" << std::endl;
        file.close();
        
        // Wait for server confirmation
        memset(buffer, 0, BUFFER_SIZE);
        bytes_read = read(sock, buffer, BUFFER_SIZE);
        
        std::string response(buffer);
        
        if (response.find("OK") != std::string::npos) {
            std::cout << "\n✓ Upload complete!" << std::endl;
            std::cout << "  File: " << filename << std::endl;
            std::cout << "  Size: " << formatFileSize(bytes_sent) 
                      << " (" << bytes_sent << " bytes)" << std::endl;
        } else {
            std::cout << "\n✗ Upload failed" << std::endl;
            std::cout << response << std::endl;
        }
    }

    void handleHelpCommand() {
        std::cout << "\n📖 Requesting help from server...\n" << std::endl;
        sendCommand("HELP\n");
        
        std::string response = receiveResponse();
        if (!response.empty()) {
            std::cout << response << std::endl;
        }
    }

    std::string normalizeCommand(const std::string& input) {
        if (input == "1") return "LIST";
        if (input == "2") return "INFO";
        if (input == "3") return "DOWNLOAD";
        if (input == "4") return "UPLOAD";
        if (input == "5") return "HELP";
        if (input == "6") return "EXIT";
        
        std::string upper = input;
        for (char& c : upper) {
            c = toupper(c);
        }
        return upper;
    }

public:
    FileClient() : sock(0), connected(false) {
        serv_addr = {};
    }

    bool connectToServer(const char* server_ip = "127.0.0.1") {
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            std::cerr << "✗ Socket creation error" << std::endl;
            return false;
        }

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(PORT);

        if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
            std::cerr << "✗ Invalid address / Address not supported" << std::endl;
            return false;
        }

        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
            std::cerr << "✗ Connection failed" << std::endl;
            std::cerr << "  Make sure the server is running on " << server_ip 
                      << ":" << PORT << std::endl;
            return false;
        }

        connected = true;
        std::cout << "✓ Connected to server at " << server_ip 
                  << ":" << PORT << std::endl;
        return true;
    }

    void run() {
        std::string welcome = receiveResponse();
        if (!welcome.empty()) {
            std::cout << "\n" << welcome;
        }

        while (connected) {
            displayMenu();
            
            std::string input;
            std::getline(std::cin, input);
            
            if (input.empty()) {
                continue;
            }

            std::string command = normalizeCommand(input);

            if (command == "LIST") {
                handleListCommand();
            }
            else if (command == "INFO") {
                handleInfoCommand();
            }
            else if (command == "DOWNLOAD") {
                handleDownloadCommand();
            }
            else if (command == "UPLOAD") {
                handleUploadCommand();
            }
            else if (command == "HELP") {
                handleHelpCommand();
            }
            else if (command == "EXIT") {
                std::cout << "\n👋 Disconnecting from server..." << std::endl;
                sendCommand("EXIT\n");
                std::string response = receiveResponse();
                if (!response.empty()) {
                    std::cout << response;
                }
                break;
            }
            else {
                std::cout << "\n❌ Unknown command. Please try again." << std::endl;
            }
        }
    }

    ~FileClient() {
        if (sock > 0) {
            close(sock);
        }
        std::cout << "\n✓ Client shutdown complete" << std::endl;
    }
};

int main(int argc, char const *argv[]) {
    std::cout << "╔════════════════════════════════════════╗" << std::endl;
    std::cout << "║   File Sharing Client (Day 4)          ║" << std::endl;
    std::cout << "╚════════════════════════════════════════╝" << std::endl;
    
    const char* server_ip = "127.0.0.1";
    
    if (argc > 1) {
        server_ip = argv[1];
    }

    FileClient client;
    
    if (client.connectToServer(server_ip)) {
        client.run();
    }

    return 0;
}

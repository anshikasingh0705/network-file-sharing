// server.cpp - File Sharing Server (Day 1: Socket Setup)
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

class FileServer {
private:
    int server_fd;
    int client_socket;
    struct sockaddr_in address;
    int addrlen;

public:
    FileServer() : server_fd(0), client_socket(0), addrlen(sizeof(address)) {
        address = {};
    }

    bool initialize() {
        // Create socket file descriptor
        if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
            perror("Socket creation failed");
            return false;
        }

        // Set socket options to reuse address
        int opt = 1;
        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                       &opt, sizeof(opt))) {
            perror("Setsockopt failed");
            return false;
        }

        // Configure address structure
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(PORT);

        // Bind socket to port
        if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
            perror("Bind failed");
            return false;
        }

        // Start listening
        if (listen(server_fd, 3) < 0) {
            perror("Listen failed");
            return false;
        }

        std::cout << "✓ Server initialized successfully" << std::endl;
        std::cout << "✓ Listening on port " << PORT << std::endl;
        return true;
    }

    void acceptConnection() {
        std::cout << "\nWaiting for client connection..." << std::endl;
        
        if ((client_socket = accept(server_fd, (struct sockaddr *)&address,
                                   (socklen_t*)&addrlen)) < 0) {
            perror("Accept failed");
            return;
        }

        // Get client information
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(address.sin_addr), client_ip, INET_ADDRSTRLEN);
        
        std::cout << "✓ Client connected from " << client_ip 
                  << ":" << ntohs(address.sin_port) << std::endl;
    }

    void handleClient() {
        char buffer[BUFFER_SIZE] = {0};
        
        // Send welcome message
        const char* welcome = "Welcome to File Sharing Server!\n";
        send(client_socket, welcome, strlen(welcome), 0);

        while (true) {
            memset(buffer, 0, BUFFER_SIZE);
            
            int bytes_read = read(client_socket, buffer, BUFFER_SIZE);
            
            if (bytes_read <= 0) {
                std::cout << "✗ Client disconnected" << std::endl;
                break;
            }

            std::cout << "Received: " << buffer;

            // Echo back for now (Day 1 testing)
            std::string response = "Server received: " + std::string(buffer);
            send(client_socket, response.c_str(), response.length(), 0);

            // Check for exit command
            if (strncmp(buffer, "EXIT", 4) == 0) {
                std::cout << "Client requested disconnection" << std::endl;
                break;
            }
        }

        close(client_socket);
    }

    void run() {
        if (!initialize()) {
            return;
        }

        // For Day 1: Handle one client at a time
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
    std::cout << "=== File Sharing Server ===" << std::endl;
    
    FileServer server;
    server.run();

    return 0;
}

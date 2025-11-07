// client.cpp - File Sharing Client (Day 1: Socket Setup)
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024

class FileClient {
private:
    int sock;
    struct sockaddr_in serv_addr;

public:
    FileClient() : sock(0) {
        serv_addr = {};
    }

    bool connectToServer(const char* server_ip = "127.0.0.1") {
        // Create socket
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            std::cerr << "✗ Socket creation error" << std::endl;
            return false;
        }

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(PORT);

        // Convert IPv4 address from text to binary
        if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
            std::cerr << "✗ Invalid address / Address not supported" << std::endl;
            return false;
        }

        // Connect to server
        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
            std::cerr << "✗ Connection failed" << std::endl;
            std::cerr << "  Make sure the server is running on " << server_ip 
                      << ":" << PORT << std::endl;
            return false;
        }

        std::cout << "✓ Connected to server at " << server_ip 
                  << ":" << PORT << std::endl;
        return true;
    }

    void run() {
        char buffer[BUFFER_SIZE] = {0};
        
        // Read welcome message from server
        int bytes_read = read(sock, buffer, BUFFER_SIZE);
        if (bytes_read > 0) {
            std::cout << buffer;
        }

        std::cout << "\nCommands:" << std::endl;
        std::cout << "  Type any message to send to server" << std::endl;
        std::cout << "  Type 'EXIT' to disconnect\n" << std::endl;

        while (true) {
            std::cout << "> ";
            std::string message;
            std::getline(std::cin, message);

            if (message.empty()) {
                continue;
            }

            // Send message to server
            send(sock, message.c_str(), message.length(), 0);

            // Check if user wants to exit
            if (message == "EXIT") {
                std::cout << "Disconnecting..." << std::endl;
                break;
            }

            // Receive response from server
            memset(buffer, 0, BUFFER_SIZE);
            bytes_read = read(sock, buffer, BUFFER_SIZE);
            
            if (bytes_read <= 0) {
                std::cout << "✗ Server disconnected" << std::endl;
                break;
            }

            std::cout << buffer << std::endl;
        }
    }

    ~FileClient() {
        if (sock > 0) {
            close(sock);
        }
        std::cout << "Client shutdown complete" << std::endl;
    }
};

int main(int argc, char const *argv[]) {
    std::cout << "=== File Sharing Client ===" << std::endl;
    
    const char* server_ip = "127.0.0.1";  // localhost by default
    
    // Allow specifying server IP as command line argument
    if (argc > 1) {
        server_ip = argv[1];
    }

    FileClient client;
    
    if (client.connectToServer(server_ip)) {
        client.run();
    }

    return 0;
}

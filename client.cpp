// client.cpp - File Sharing Client (Day 2: File Listing)
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include <sstream>

#define PORT 8080
#define BUFFER_SIZE 4096

class FileClient {
private:
    int sock;
    struct sockaddr_in serv_addr;
    bool connected;

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
        std::cout << "║  1. LIST   - List all files            ║" << std::endl;
        std::cout << "║  2. INFO   - Get file information      ║" << std::endl;
        std::cout << "║  3. HELP   - Show server commands      ║" << std::endl;
        std::cout << "║  4. EXIT   - Disconnect                ║" << std::endl;
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

    void handleHelpCommand() {
        std::cout << "\n📖 Requesting help from server...\n" << std::endl;
        sendCommand("HELP\n");
        
        std::string response = receiveResponse();
        if (!response.empty()) {
            std::cout << response << std::endl;
        }
    }

    std::string normalizeCommand(const std::string& input) {
        // Convert menu numbers to commands
        if (input == "1") return "LIST";
        if (input == "2") return "INFO";
        if (input == "3") return "HELP";
        if (input == "4") return "EXIT";
        
        // Convert to uppercase for consistency
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
        // Create socket
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            std::cerr << "✗ Socket creation error" << std::endl;
            return false;
        }

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(PORT);

        // Convert address
        if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
            std::cerr << "✗ Invalid address / Address not supported" << std::endl;
            return false;
        }

        // Connect
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
        // Read welcome message
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
    std::cout << "║   File Sharing Client (Day 2)          ║" << std::endl;
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

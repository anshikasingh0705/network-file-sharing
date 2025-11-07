# Makefile for File Sharing Application

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pthread
LDFLAGS = -pthread

# Targets
SERVER = server
CLIENT = client

# Source files
SERVER_SRC = server.cpp
CLIENT_SRC = client.cpp

# Build all
all: $(SERVER) $(CLIENT)
	@echo "✓ Build complete!"
	@echo "  Run './server' in one terminal"
	@echo "  Run './client' in another terminal"

# Build server
$(SERVER): $(SERVER_SRC)
	$(CXX) $(CXXFLAGS) -o $(SERVER) $(SERVER_SRC) $(LDFLAGS)
	@echo "✓ Server compiled"

# Build client
$(CLIENT): $(CLIENT_SRC)
	$(CXX) $(CXXFLAGS) -o $(CLIENT) $(CLIENT_SRC) $(LDFLAGS)
	@echo "✓ Client compiled"

# Clean build files
clean:
	rm -f $(SERVER) $(CLIENT)
	@echo "✓ Clean complete"

# Run server
run-server: $(SERVER)
	./$(SERVER)

# Run client
run-client: $(CLIENT)
	./$(CLIENT)

# Test setup
test: all
	@echo "Starting test..."
	@echo "Open two terminals:"
	@echo "  Terminal 1: make run-server"
	@echo "  Terminal 2: make run-client"

.PHONY: all clean run-server run-client test

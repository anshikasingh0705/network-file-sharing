🔐 Secure Network File Sharing System
A complete TCP-based file sharing application with client-server architecture, featuring user authentication, role-based access control, and bidirectional file transfer.

Show Image
Show Image
Show Image

📋 Table of Contents
Overview
Features
Architecture
Prerequisites
Installation
Usage
User Accounts
Project Structure
Development Timeline
Technologies
Screenshots
Future Enhancements
Author
🎯 Overview
This project is a secure file sharing system built as part of a capstone project (Assignment 4: Network File Sharing). It implements a client-server model using TCP sockets, allowing multiple users to upload and download files with proper authentication and authorization.

Key Highlights
1900+ lines of production-quality C++ code
5-day development following structured milestones
Multi-user support with role-based permissions
Comprehensive logging for security auditing
Bidirectional file transfer with progress tracking
✨ Features
🔒 Security
User Authentication: Username/password login system
Access Control: Role-based permissions (upload/download rights)
Session Management: Secure login/logout functionality
Password Masking: Hidden password input on client
Activity Logging: All operations logged with timestamps
📁 File Operations
File Listing: Browse available files on server
File Information: Detailed file metadata (size, permissions, type)
Download: Transfer files from server to client
Upload: Transfer files from client to server
Progress Tracking: Real-time progress bars for transfers
🌐 Network
TCP Sockets: Reliable connection-oriented communication
Chunked Transfer: Efficient handling of large files (4KB chunks)
Error Handling: Robust network error management
Multiple Connections: Sequential client handling
🏗️ Architecture
┌─────────────────┐                    ┌─────────────────┐
│                 │    TCP Socket      │                 │
│     Client      │ ◄───────────────► │     Server      │
│   Application   │   Port 8080        │   Application   │
│                 │                    │                 │
└────────┬────────┘                    └────────┬────────┘
         │                                      │
         │                                      │
    ┌────▼────┐                            ┌───▼────┐
    │uploads/ │                            │shared_ │
    │downloads/│                           │files/  │
    └─────────┘                            └────────┘
                                           │users.txt│
                                           │server.log│
                                           └─────────┘
Communication Protocol
Client                          Server
  |                               |
  |--- LOGIN user:pass --------->|
  |<-- OK/ERROR -----------------| (Authentication)
  |                               |
  |--- LIST -------------------->|
  |<-- File List ----------------| (Browse files)
  |                               |
  |--- DOWNLOAD file.txt ------->|
  |<-- Metadata -----------------| (File size, name)
  |--- READY ------------------->|
  |<-- File Data (chunks) -------| (Transfer)
  |                               |
  |--- UPLOAD file.txt --------->|
  |<-- READY --------------------| (Ready to receive)
  |--- Metadata ---------------->| (File info)
  |<-- READY --------------------| (Confirmed)
  |--- File Data (chunks) ------>| (Transfer)
  |<-- OK/ERROR -----------------| (Confirmation)
📦 Prerequisites
System Requirements
Operating System: Linux (Ubuntu 20.04+ recommended) or WSL2 on Windows
Compiler: g++ with C++17 support
Build Tools: make
Libraries: Standard C++ libraries, POSIX sockets
Installation of Dependencies
Ubuntu/Debian:

bash
sudo apt update
sudo apt install build-essential git
Fedora/RHEL:

bash
sudo dnf install gcc-c++ make git
Windows (WSL):

powershell
wsl --install
# Then install Ubuntu and run the Ubuntu commands above
🚀 Installation
1. Clone the Repository
bash
git clone https://github.com/anshikasingh0705/network-file-sharing.git
cd network-file-sharing
2. Compile the Project
bash
make all
Expected output:

✓ Server compiled
✓ Client compiled
✓ Build complete!
3. Create Required Directories
bash
mkdir -p shared_files uploads downloads
4. Add Sample Files (Optional)
bash
echo "Sample file content" > shared_files/sample.txt
echo "Test upload file" > uploads/test.txt
💻 Usage
Starting the Server
Terminal 1:

bash
./server
Expected output:

=== Secure File Sharing Server (Day 5) ===
✓ Created default users file
  Default users: admin/admin123, user/user123, uploader/upload123
✓ Loaded 3 users
✓ Server initialized successfully
✓ Listening on port 8080
✓ Shared directory: ./shared_files
✓ Logging to: ./server.log

Waiting for client connection...
Connecting a Client
Terminal 2:

bash
./client
Or connect to a remote server:

bash
./client 192.168.1.100
Client Commands
After starting the client, you'll see a login menu:

╔════════════════════════════════════════╗
║    Secure File Sharing - Login         ║
╠════════════════════════════════════════╣
║  1. LOGIN  - Authenticate              ║
║  2. HELP   - Show commands             ║
║  3. EXIT   - Disconnect                ║
╚════════════════════════════════════════╝
After login:

╔════════════════════════════════════════╗
║    File Sharing Client - Menu          ║
║    Logged in as: admin                 ║
╠════════════════════════════════════════╣
║  1. LIST     - List server files       ║
║  2. INFO     - Get file information    ║
║  3. DOWNLOAD - Download from server    ║
║  4. UPLOAD   - Upload to server        ║
║  5. LOGOUT   - Logout                  ║
║  6. HELP     - Show commands           ║
║  7. EXIT     - Disconnect              ║
╚════════════════════════════════════════╝
👥 User Accounts
The system comes with 3 pre-configured accounts:

UsernamePasswordUploadDownloadDescription
adminadmin123✅✅Full access - can upload and download
users.txt user123❌✅Read-only - can only download files
uploaderupload123✅❌Write-only - can only upload files
Adding New Users
Edit the users.txt file:

bash
nano users.txt
Format: username:password:can_upload(0/1):can_download(0/1)

Example:

newuser:mypass:1:1
readonly:pass123:0:1
writeonly:secure456:1:0
Restart the server to load new users.

📂 Project Structure
file_sharing_system/
├── server.cpp              # Server implementation (1000+ lines)
├── client.cpp              # Client implementation (900+ lines)
├── Makefile                # Build configuration
├── README.md               # This file
├── .gitignore              # Git ignore rules
│
├── shared_files/           # Server's shared directory
│   ├── sample.txt
│   └── data.bin
│
├── uploads/                # Client's upload directory
│   └── file_to_upload.txt
│
├── downloads/              # Client's download directory
│   └── downloaded_file.txt
│
├── users.txt               # User database (auto-generated)
├── server.log              # Activity log (auto-generated)
│
└── backups/                # Development backups
    ├── day1_backup/
    ├── day2_backup/
    ├── day3_backup/
    └── day4_backup/
📅 Development Timeline
Day 1: Socket Communication ✅
Basic TCP server implementation
Client connection handling
Message passing between client and server
Lines Added: ~250
Day 2: File Listing & Protocol Design ✅
LIST command implementation
INFO command for file details
Protocol structure (OK/ERROR responses)
Menu-driven client interface
Lines Added: ~300
Day 3: File Download ✅
DOWNLOAD command
Chunked file transfer (4KB chunks)
Progress bar visualization
File metadata protocol
Lines Added: ~350
Day 4: File Upload ✅
UPLOAD command
Bidirectional file transfer
Local file browser
Upload progress tracking
Lines Added: ~300
Day 5: Security & Authentication ✅
User authentication system
Role-based access control
Activity logging
Password masking
Session management
Lines Added: ~400
Total: 1900+ lines of C++ code

🛠️ Technologies
Programming Language
C++17: Modern C++ features, STL containers
Libraries & APIs
POSIX Sockets: TCP/IP networking
Standard C++ Library: I/O streams, strings, containers
POSIX System Calls: File operations, stat, directory traversal
Termios: Terminal I/O control (password masking)
Build System
GNU Make: Automated compilation
Version Control
Git: Source code management
GitHub: Repository hosting
📸 Screenshots
Server Running
=== Secure File Sharing Server (Day 5) ===
✓ Loaded 3 users
✓ Server initialized successfully
✓ Listening on port 8080
✓ Shared directory: ./shared_files
✓ Logging to: ./server.log

Waiting for client connection...
Client Login
🔐 Login to Secure File Server
----------------------------------------
Username: admin
Password: ********

🔄 Authenticating...

OK
Login successful! Welcome, admin
Permissions:
  - Upload: YES
  - Download: YES

✓ Authentication successful!
File Transfer with Progress
📥 Requesting download: large_file.bin
File size: 10.50 MB (11010048 bytes)
Saving to: ./downloads/large_file.bin

🔄 Downloading...
Progress: [==========================                    ] 52%
Server Activity Log
[Sat Nov 08 14:23:45 2025] [127.0.0.1] [ANONYMOUS] CONNECTED
[Sat Nov 08 14:23:50 2025] [127.0.0.1] [admin] LOGIN SUCCESS
[Sat Nov 08 14:23:55 2025] [127.0.0.1] [admin] LIST - 8 items
[Sat Nov 08 14:24:10 2025] [127.0.0.1] [admin] DOWNLOAD - report.pdf (2048 bytes)
[Sat Nov 08 14:24:30 2025] [127.0.0.1] [admin] UPLOAD - data.csv (15360 bytes)
[Sat Nov 08 14:25:00 2025] [127.0.0.1] [admin] LOGOUT
🚀 Future Enhancements
Planned Features
 Multi-threading: Support multiple simultaneous clients
 SSL/TLS Encryption: Secure data transmission
 Password Hashing: SHA-256 or bcrypt for passwords
 Database Integration: SQLite for user management
 GUI Interface: Qt or GTK-based client
 File Compression: Compress files before transfer
 Resume Support: Resume interrupted transfers
 Bandwidth Throttling: Control transfer speeds
 Directory Upload: Upload entire folders
 File Search: Search files by name or content
Potential Improvements
Add unit tests (Google Test framework)
Implement retry mechanism for failed transfers
Add file versioning support
Create web-based client interface
Add support for file sharing links
Implement user groups and shared folders
🧪 Testing
Running Tests
Test Authentication:

bash
# Terminal 1: Start server
./server

# Terminal 2: Test login
./client
# Try: admin/admin123 (should work)
# Try: admin/wrongpass (should fail)
Test Permissions:

bash
# Login as 'user' (download-only)
# Try to upload - should be denied
# Try to download - should work

# Login as 'uploader' (upload-only)
# Try to download - should be denied
# Try to upload - should work
Test File Integrity:

bash
# Upload a file
echo "test content" > uploads/test.txt
./client
# Login and upload test.txt

# Download it back
# Login and download test.txt

# Compare
diff uploads/test.txt downloads/test.txt
# Should show no differences
View Logs:

bash
# Check all activity
cat server.log

# Watch logs in real-time
tail -f server.log
🐛 Troubleshooting
Issue: Port 8080 already in use
bash
# Find process using port
sudo lsof -i :8080

# Kill the process
kill -9 <PID>

# Or change port in code (both server.cpp and client.cpp)
#define PORT 8081
Issue: Permission denied
bash
# Make executables
chmod +x server client

# Ensure directories exist and are writable
mkdir -p shared_files uploads downloads
chmod 755 shared_files uploads downloads
Issue: Client can't connect
bash
# Check if server is running
ps aux | grep server

# Check if port is listening
netstat -tuln | grep 8080

# Try explicit IP
./client 127.0.0.1
Issue: Compilation errors
bash
# Clean and rebuild
make clean
make all

# Check g++ version (need C++17 support)
g++ --version
# Should be 7.0 or higher
📝 License
This project is developed as part of a capstone assignment. Feel free to use for educational purposes.

👤 Author
Anshika Singh

GitHub: @anshikasingh0705
Project: network-file-sharing
🙏 Acknowledgments
Capstone Project: Assignment 4 - Network File Sharing Server & Client
Technologies: Linux, C++, POSIX Sockets, TCP/IP
Development Period: November 2025
Status: ✅ Complete - All 5 days implemented
📚 Documentation
For more detailed information:

System Architecture
API Documentation
Protocol Specification
Development Log
🎯 Project Goals Achieved
✅ Implement client-server architecture using sockets
✅ Enable bidirectional file transfer
✅ Add user authentication system
✅ Implement role-based access control
✅ Create comprehensive activity logging
✅ Handle errors gracefully
✅ Support multiple file operations
✅ Provide user-friendly interface
✅ Write clean, maintainable code
✅ Document thoroughly
⭐ If you found this project helpful, please star the repository!

Last Updated: November 2025


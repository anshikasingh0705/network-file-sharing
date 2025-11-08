# 🔐 Secure Network File Sharing System

A complete TCP-based file sharing application with client-server architecture, featuring user authentication, role-based access control, and bidirectional file transfer.

## 📋 Overview

This project is a secure file sharing system built as part of Assignment 4: Network File Sharing. It implements a client-server model using TCP sockets, allowing multiple users to upload and download files with proper authentication and authorization.

### Key Features
- **1900+ lines** of production-quality C++ code
- **User Authentication** with username/password
- **Role-based Permissions** (upload/download rights)
- **Bidirectional File Transfer** with progress tracking
- **Activity Logging** for security auditing

## 🚀 Installation
```bash
# Clone the repository
git clone https://github.com/anshikasingh0705/network-file-sharing.git
cd network-file-sharing

# Compile
make all

# Create directories
mkdir -p shared_files uploads downloads
```

## 💻 Usage

### Start Server
```bash
./server
```

### Start Client
```bash
./client
```

## 👥 Default User Accounts

| Username | Password | Upload | Download |
|----------|----------|--------|----------|
| admin | admin123 | ✅ | ✅ |
| user | user123 | ❌ | ✅ |
| uploader | upload123 | ✅ | ❌ |

## 📂 Project Structure
```
file_sharing_system/
├── server.cpp       # Server (1000+ lines)
├── client.cpp       # Client (900+ lines)
├── Makefile
├── shared_files/    # Server files
├── uploads/         # Client upload folder
├── downloads/       # Client download folder
├── users.txt        # User database
└── server.log       # Activity log
```

## 🛠️ Technologies

- **C++17** - Modern C++
- **POSIX Sockets** - TCP/IP networking
- **Linux** - System calls and APIs

## 📅 Development Timeline

- **Day 1**: Socket communication
- **Day 2**: File listing & protocol
- **Day 3**: Download functionality
- **Day 4**: Upload functionality
- **Day 5**: Authentication & security

## 🎯 Features

### Security
- User authentication
- Role-based access control
- Session management
- Password masking
- Activity logging

### File Operations
- List files
- File information
- Download files
- Upload files
- Progress tracking

## 👤 Author

**Anshika Singh**
- GitHub: [@anshikasingh0705](https://github.com/anshikasingh0705)

## 📝 License

Educational project - part of capstone assignment.

---

*Assignment 4: Network File Sharing Server & Client*

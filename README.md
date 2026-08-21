# RCE - Remote Code Execution

A lightweight, cross-platform framework for securely executing code on remote servers. Designed for controlled environments, private networks, and authorized remote task execution. This is **v1.0 - Proof of Concept**.

> **Warning:** By design, this application allows arbitrary code execution on the server host. It is strictly intended for controlled environments, private networks, or authorized remote task execution. Do not expose to untrusted networks.

---

## Features

- ✅ **Cross-Platform Support** — Windows, Linux, macOS
- ✅ **wxWidgets GUI** — Easy-to-use client and server interface
- ✅ **Bidirectional Communication** — File transfer + execution output return
- ✅ **Reliable Protocol** — Custom wire protocol with payload validation
- ✅ **Real-time Output** — Stdout/stderr streamed back to client
- ✅ **Temporary File Cleanup** — Automatic removal after execution

---

## Requirements

### Dependencies
- **C++17** or later
- **wxWidgets 3.1+** (for GUI)
- **CMake 3.15+** (for building)
- **Compiler** — GCC, Clang, or MSVC

### Installation

**Ubuntu/Debian:**
```bash
sudo apt-get install libwxgtk3.0-gtk3-dev cmake build-essential
```

**macOS:**
```bash
brew install wxwidgets cmake
```

**Windows:**
- Download wxWidgets from https://www.wxwidgets.org/
- Set `wxWidgets_ROOT_DIR` environment variable
- Install Visual Studio Build Tools or MSVC compiler
- Install CMake for Windows

---

## Building

```bash
# Clone the repository
git clone https://github.com/YsnBnc/rce.git
cd rce

# Create build directory
mkdir build && cd build

# Configure with CMake
cmake ..

# Build
cmake --build . --config Release

# Binary output: ../release/RCE (or RCE.exe on Windows)
```

### Build Options

```bash
# Debug build with symbols
cmake -DCMAKE_BUILD_TYPE=Debug ..

# Release build (optimized)
cmake -DCMAKE_BUILD_TYPE=Release ..
```

---

## Usage

### Running the Application

```bash
./RCE  # Linux/macOS
RCE.exe  # Windows
```

The GUI will open with two tabs: **Server Side** and **Client Side**.

### Server Mode

**Purpose:** Listen for incoming connections and execute commands sent by clients.

**Steps:**
1. Click **Sides → Server Side** menu
2. Enter a **PORT** (e.g., `8888`)
3. Click **EXECUTE** button
4. Server waits for connections (terminal shows "Listening for connections...")
5. When a client connects:
   - Receives file + compile command
   - Writes file to disk
   - Executes the command
   - Sends output back to client
   - Cleans up temporary file

**Example:**
```
Port: 8888
[EXECUTE]

Output:
Server starting...
Socket created.
Bind successful.
Listening for connections...
Connection established.
Data recieved.
Received file: test.cpp
Compiling file
File compiled
```

### Client Mode

**Purpose:** Connect to a remote server, send code to compile/execute, receive results.

**Steps:**
1. Click **Sides → Client Side** menu
2. Enter server **PORT** (e.g., `8888`)
3. Enter server **IP** (e.g., `192.168.1.100`)
4. Enter **COMMAND** to execute (e.g., `g++ test.cpp -o test`)
5. Click **BROWSE** to select a file
6. Click **EXECUTE** to send
7. Wait for output to appear in terminal

**Example:**
```
Port: 8888
Server IP: 192.168.1.100
Command: g++ test.cpp -o test

[BROWSE] → select test.cpp
[EXECUTE]

Output:
Client Side started on 192.168.1.100:8888
Sent data:
Sent file: test.cpp
Data recieved.
Response from server:
[compilation successful output...]
```

---

## Protocol

### Wire Format

The RCE protocol uses a simple **header + payload** structure:

#### WireHeader (6 bytes)
```c
struct WireHeader {
  uint32_t payload_length;  // Length of payload (network byte order)
  uint16_t msg_type;        // Message type (0x0100 or 0x0200)
};
```

#### Message Types
- `0x0100` — Client-to-Server (file submission)
- `0x0200` — Server-to-Client (response/output)

#### Client-to-Server Payload
```
[4 bytes]  FILE_INDEX (uint32)
[2 bytes]  FILE_NAME length (uint16)
[variable] FILE_NAME
[2 bytes]  COMPILE_COMMAND length (uint16)
[variable] COMPILE_COMMAND
[4 bytes]  FILE_CONTENT length (uint32)
[variable] FILE_CONTENT
```

#### Server-to-Client Payload
```
[4 bytes]  RESPONSE_INDEX (uint32)
[2 bytes]  RESPONSE length (uint16)
[variable] RESPONSE (compilation output/errors)
```

All multi-byte integers are in **network byte order** (big-endian) using `htonl()`/`htons()`.

### Limitations

- **Max Filename:** 65,535 bytes (uint16)
- **Max Command:** 65,535 bytes (uint16)
- **Max File Size:** 4 GB (uint32)
- **Max Payload:** 64 MB (safety limit)

---

## Architecture

### Directory Structure

```
rce/
├── src/
│   ├── main.cpp              # GUI app, wxWidgets frame, event handlers
│   └── utilities/
│       ├── bridge.h          # Protocol definitions, function declarations
│       ├── client_side.cpp    # Client socket logic, sending files
│       ├── server_side.cpp    # Server socket logic, receiving & executing
│       └── file_manager.cpp   # Packing, serialization, compilation
├── CMakeLists.txt            # Build configuration
├── LICENSE                   # MIT License
└── README.md                 # This file
```

### Key Components

| Component | Purpose |
|-----------|---------|
| **main.cpp** | wxWidgets GUI, tab switching, input handling |
| **client_side.cpp** | Socket connection, file packing, sending, receiving response |
| **server_side.cpp** | Socket listening, receiving payload, deserializing data |
| **file_manager.cpp** | Serialization (pack_file), execution (compile_file), I/O |
| **bridge.h** | Protocol definitions, function prototypes, WireHeader struct |

### Communication Flow

```
1. User selects file + enters command in GUI
   ↓
2. pack_file() serializes data into binary payload
   ↓
3. WireHeader created (6 bytes: length + type)
   ↓
4. send_exact() sends header + payload over TCP
   ↓
5. Server recv_exact() reads header, knows payload size
   ↓
6. Server deserializes, writes file, executes command via popen()
   ↓
7. pack_response() serializes compilation output
   ↓
8. Server sends response header + payload
   ↓
9. Client recv_exact() receives and deserializes
   ↓
10. GUI displays output to user
```

---

## Examples

### Example 1: Compile C++ on Remote Server

**Client Setup:**
```
Port: 9000
Server IP: 10.0.0.50
Command: g++ -std=c++17 main.cpp -o main
File: /home/user/main.cpp
```

**Server:** Running on `10.0.0.50:9000`

**Result:** Compiled binary `main` created on server, output sent to client GUI

---

### Example 2: Run Python Script

**Client Setup:**
```
Port: 9000
Server IP: 10.0.0.50
Command: python3 script.py
File: /path/to/script.py
```

**Server Output:**
```
Script execution result displayed in client terminal
```

---

### Example 3: Execute Bash Commands

**Client Setup:**
```
Port: 9000
Server IP: 10.0.0.50
Command: bash test.sh
File: /path/to/test.sh
```

**Server:** Executes bash script, streams output to client

---

## Platform-Specific Notes

### Windows
- Requires absolute file paths (e.g., `C:\Users\user\file.cpp`)
- Uses `WSAStartup()`/`WSACleanup()` for socket initialization
- Uses `_popen()`/`_pclose()` for command execution
- Built with `-static-libgcc -static-libstdc++ -static` flags

### Linux/macOS
- Supports relative and absolute file paths
- Uses POSIX socket API
- Uses `popen()`/`pclose()` for command execution
- Requires `libwxgtk3.0-gtk3` on Linux

---

## Troubleshooting

### "Connect failed"
- Verify server is running and listening on the specified port
- Check firewall settings
- Ensure server IP is correct (use `ifconfig` or `ipconfig` to find)

### "Bind failed"
- Port is already in use. Try a different port (8000-9000 range is safe)
- On Linux, may need `sudo` for ports < 1024

### "Failed to execution pipeline"
- Command not found on server (e.g., `g++` not installed)
- Verify compiler is in PATH on server machine

### File not compiling
- Check command syntax is correct for the server's OS
- Ensure file path in command matches the uploaded filename
- Check compilation output for specific errors

### Windows: "File path must be absolute"
- Provide full path like `C:\Users\YourName\project\main.cpp`
- Do NOT use relative paths on Windows client

---

## Limitations & Future Improvements

### Current Limitations (v1.0)
- Single connection per server session (connect, execute, disconnect)
- No encryption (use only on private networks)
- No authentication (assumes trusted network)
- No persistent logging
- 64 MB payload size limit

### Planned for v2.0 (Commercial)
- Multi-threaded server (concurrent clients)
- TLS/SSL encryption
- Client authentication
- Job queue & scheduling
- Persistent execution logs
- Output streaming (real-time stdout)
- Binary upload support
- Remote shell access

---

## License

This project is licensed under the **MIT License** — see [LICENSE](LICENSE) file for details.

---

## Author

**YsnBnc** — https://github.com/YsnBnc

---

## Support & Issues

Found a bug? Have a suggestion?

1. Check [Issues](https://github.com/YsnBnc/rce/issues) for existing reports
2. Open a new issue with:
   - OS & platform (Windows/Linux/macOS)
   - Build output & error messages
   - Steps to reproduce
   - Expected vs actual behavior

---

## Safety & Security

⚠️ **This tool grants arbitrary code execution.** Use ONLY in:
- ✅ Controlled development environments
- ✅ Private networks with trusted users
- ✅ Authorized enterprise systems
- ✅ Proof-of-concept testing

❌ **DO NOT use** on:
- Public internet
- Untrusted networks
- Production systems without hardening
- Systems with sensitive data

For production deployments, v2.0 commercial version will include enterprise security features.

---

## Quick Start (TL;DR)

```bash
# Build
mkdir build && cd build && cmake .. && cmake --build .

# Terminal 1: Start server on port 8888
cd release && ./RCE
# → Click: Sides → Server Side → Enter 8888 → Click EXECUTE

# Terminal 2: Send file from client
cd release && ./RCE
# → Click: Sides → Client Side
# → Port: 8888, IP: localhost (or 127.0.0.1)
# → Browse: select test.cpp
# → Command: g++ test.cpp -o test
# → Click: EXECUTE

# Output appears in client GUI terminal
```

Enjoy! 🚀

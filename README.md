# RCE - Remote Code Execution

A lightweight, cross-platform framework for securely executing code on remote servers. Designed for controlled environments, private networks, and authorized remote task execution. This is **v1.0 - Proof of Concept**.

> **Warning:** By design, this application allows arbitrary code execution on the server host. It is strictly intended for controlled environments, private networks, or authorized remote task execution. Do not expose to untrusted networks.

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
**Arch:**
```bash
sudo pacman -S base-devel cmake wxgtk3
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

## Troubleshooting

### Windows: Missing DLL error when trying to run executable
- Create `resource.rc` file in project directory
- Paste following lines
```bash
#include <windows.h>
1 24 "wx/msw/wx.manifest"
```
- Go to `CMakeLists.txt` and add `resource.rc` to  `add_executable` section
```cmake
add_executable(RCE
        src/main.cpp
        src/utilities/client_side.cpp
        src/utilities/server_side.cpp
        src/utilities/file_manager.cpp
        resource.rc
)
```
- Try to build 
---

## Limitations & Future Improvements

### Current Limitations (v1.0)
- Single connection per server session (connect, execute, disconnect)
- No encryption (use only on private networks)
- No authentication (assumes trusted network)
- No persistent logging
- 64 MB payload size limit
---

## License

This project is licensed under the **MIT License** — see [LICENSE](LICENSE) file for details.

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

**YsnBnc** — https://github.com/YsnBnc


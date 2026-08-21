# RCE Protocol Flowchart & Wire Format

## 1. Complete Message Flow (Client to Server to Client)

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                          REMOTE CODE EXECUTION                              │
│                         (RCE Protocol v1.0 Flow)                            │
└─────────────────────────────────────────────────────────────────────────────┘

CLIENT SIDE                              SERVER SIDE
═══════════════════════════════════════════════════════════════════════════════

┌──────────────────────┐
│   User Opens GUI     │
│  (RCE_Frame loaded)  │
└──────────────────────┘
         │
         ├─────> Switch to CLIENT_SIDE tab
         │
         ▼
┌──────────────────────────────────────────┐
│  Input Parameters:                       │
│  • Server IP: 192.168.1.100              │
│  • Port: 8888                            │
│  • File: /path/to/main.cpp               │
│  • Command: g++ main.cpp -o main         │
└──────────────────────────────────────────┘
         │
         ▼
┌──────────────────────────────────────────┐
│  Click EXECUTE Button                    │
│  (onClientExecuteClicked triggered)      │
└──────────────────────────────────────────┘
         │
         ▼
┌──────────────────────────────────────────┐
│  Extract GUI inputs:                     │
│  • Parse IP from ipInput                 │
│  • Parse PORT from client_portInput      │
│  • Get COMPILE_COMMAND from commandInput │
│  • Read FILE_CONTENT from disk           │
│  • Extract FILE_NAME from path           │
└──────────────────────────────────────────┘
         │
         ▼
┌──────────────────────────────────────────┐
│  call client.client_side(                │
│      PORT=8888,                          │
│      TARGET_IP="192.168.1.100",          │
│      FILE_INDEX=42,                      │
│      FILE_NAME="main.cpp",               │
│      COMPILE_COMMAND="g++ ...",          │
│      FILE_CONTENT="<file bytes>")        │
└──────────────────────────────────────────┘
         │
         ▼
┌───────���──────────────────────────────────┐
│  Socket Setup:                           │
│  • Create AF_INET, SOCK_STREAM socket    │
│  • Set server_address struct             │
│  • inet_pton() convert IP string         │
│  • htons() convert port to network order │
└──────────────────────────────────────────┘
         │
         ▼
┌──────────────────────────────────────────┐
│  connect() to server                     │
│  (blocks until connection accepted)      │
└──────────────────────────────────────────┘
         │
         │ ┌─── Connection established ───┐
         │ │                              │
         │ ▼                              ▼
         │                    ┌──────────────────────────┐
         │                    │ Server listening on 8888 │
         │                    │ (server.server_side())   │
         │                    │ accept() blocking        │
         │                    └──────────────────────────┘
         │                              ▲
         │                              │
         │ ┌───────────────────────────┐│
         │ │ Connection accepted!      ││
         │ │ newSocket created         ││
         │ └───────────────────────────┘│
         │                              │
         ▼ ◄─────────────────────────────┘

═════════════════════════════════════════════════════════════════════════════

                    PHASE 1: CLIENT → SERVER

═════════════════════════════════════════════════════════════════════════════

CLIENT                                   SERVER
───────────────────────────────────────────────────────────────────────────

┌──────────────────────────────────────┐
│ pack_file() called:                  │
│                                      │
│ Serializes:                          │
│ 1. FILE_INDEX (uint32)       [4B]    │
│ 2. FILE_NAME length (uint16) [2B]    │
│ 3. FILE_NAME content        [var]    │
│ 4. CMD length (uint16)       [2B]    │
│ 5. COMMAND content          [var]    │
│ 6. FILE length (uint32)      [4B]    │
│ 7. FILE content             [var]    │
│                                      │
│ Returns: std::vector<uint8_t>        │
│          total size = X bytes        │
└──────────────────────────────────────┘
         │
         ▼
┌──────────────────────────────────────┐
│ Create WireHeader struct:            │
│                                      │
│ struct WireHeader {                  │
│   uint32_t payload_length;           │
│          = htonl(X);  // X bytes     │
│   uint16_t msg_type;                 │
│          = htons(0x0100);            │
│ };                                   │
│ Size = 6 bytes                       │
└──────────────────────────────────────┘
         │
         ▼
┌──────────────────────────────────────┐
│ send_exact(socket, &header, 6):      │
│                                      │
│ Loop: while (sent < 6) {             │
│   send(socket, ptr+sent, 6-sent)     │
│   Update sent counter                │
│ }                                    │
│                                      │
│ Sends 6 bytes over TCP               │
└──────────────────────────────────────┘
         │
         │ ┌─── 6 bytes over network ───┐
         │ │  (fragmented by TCP OK)    │
         │ ▼                            ▼
         │                   ┌──────────────────────────┐
         │                   │ recv_exact() listening   │
         │                   │ Expects WireHeader (6B)  │
         │                   │ memcpy into struct       │
         │                   │ payload_len = ntohl(X)   │
         │                   │ msg_type = ntohs(0x0100) │
         │                   │ → "Client-to-Server"     │
         │                   └──────────────────────────┘
         │                              ▲
         ▼                              │

┌──────────────────────────────────────┐
│ send_exact(socket, payload, X):      │
│                                      │
│ Loop: while (sent < X) {             │
│   send(socket, ptr+sent, X-sent)     │
│   Update sent counter                │
│ }                                    │
│                                      │
│ Sends X bytes (payload) over TCP     │
└──────────────────────────────────────┘
         │
         │ ┌─── X bytes over network ───┐
         │ │  (may arrive in chunks)    │
         │ ▼                            ▼
         │                   ┌──────────────────────────┐
         │                   │ recv_exact() listening   │
         │                   │ Expects X bytes          │
         │                   │ Receives all X bytes     │
         │                   │ Stores in payload_buffer │
         │                   └──────────────────────────┘
         │                              ▲
         ▼                              │

┌──────────────────────────────────────┐
│ Print to GUI:                        │
│ "Sent file: main.cpp"                │
│ (waiting for response...)            │
│                                      │
│ Now recv_exact() for response header │
└──────────────────────────────────────┘
         │                              │
         │ ┌───────────────────────────┘
         │ │
         ▼ ▼

═════════════════════════════════════════════════════════════════════════════

                    PHASE 2: SERVER PROCESSES

═════════════════════════════════════════════════════════════════════════════

SERVER (after receiving all payload bytes)
────────────────────────────────────────────────────────────────────────────

┌──────────────────────────────────────┐
│ Deserialize payload:                 │
│ (offset = 0)                         │
│                                      │
│ 1. Read FILE_INDEX (offset 0-3)      │
│    memcpy() 4 bytes                  │
│    ntohl() convert to host order     │
│    offset += 4                       │
│                                      │
│ 2. Read FILE_NAME length (offset 4-5)
│    memcpy() 2 bytes                  │
│    ntohs() convert                   │
│    offset += 2                       │
│                                      │
│ 3. Read FILE_NAME content            │
│    memcpy() fn_len bytes             │
│    offset += fn_len                  │
│    FILE_NAME.assign()                │
│                                      │
│ 4. Read COMPILE_COMMAND length (2B)  │
│    memcpy() 2 bytes                  │
│    ntohs() convert                   │
│    offset += 2                       │
│                                      │
│ 5. Read COMPILE_COMMAND content      │
│    memcpy() cmd_len bytes            │
│    offset += cmd_len                 │
│    COMPILE_COMMAND.assign()          │
│                                      │
│ 6. Read FILE_CONTENT length (4B)     │
│    memcpy() 4 bytes                  │
│    ntohl() convert                   │
│    offset += 4                       │
│                                      │
│ 7. Read FILE_CONTENT (var bytes)     │
│    memcpy() ct_len bytes             │
│    offset += ct_len                  │
│    FILE_CONTENT.assign()             │
└──────────────────────────────────────┘
         │
         ▼
┌──────────────────────────────────────┐
│ Extracted variables:                 │
│ • FILE_INDEX = 42                    │
│ • FILE_NAME = "main.cpp"             │
│ • COMPILE_COMMAND = "g++ ..."        │
│ • FILE_CONTENT = "#include ..."      │
└──────────────────────────────────────┘
         │
         ▼
┌──────────────────────────────────────┐
│ write_file(FILE_NAME, FILE_CONTENT): │
│                                      │
│ Create temporary file:               │
│ std::ofstream file("main.cpp")       │
│ file << FILE_CONTENT                 │
│ file.close()                         │
│                                      │
│ Result: main.cpp created on server   │
└──────────────────────────────────────┘
         │
         ▼
┌──────────────────────────────────────┐
│ compile_file(COMPILE_COMMAND):       │
│                                      │
│ cmd = "g++ main.cpp -o main 2>&1"    │
│ (append 2>&1 to capture stderr)      │
│                                      │
│ FILE *pipe = popen(cmd, "r")         │
│                                      │
│ while(fgets(buffer) != nullptr) {    │
│   output += buffer                   │
│ }                                    │
│                                      │
│ pclose(pipe)                         │
│                                      │
│ Returns: compilation output string   │
│ Example:                             │
│ "main.cpp:5: error: ..."             │
│ OR                                   │
│ "[successful compilation]"           │
└──────────────────────────────────────┘
         │
         ▼
┌──────────────────────────────────────┐
│ remove(FILE_NAME):                   │
│                                      │
│ Delete temporary file                │
│ main.cpp deleted from server         │
│ (cleanup)                            │
└──────────────────────────────────────┘
         │
         ▼

═════════════════════════════════════════════════════════════════════════════

                    PHASE 3: SERVER → CLIENT

═════════════════════════════════════════════════════════════════════════════

SERVER                                  CLIENT
────────────────────────────────────────────────────────────────────────────

┌──────────────────────────────────────┐
│ pack_response(41, output):           │
│                                      │
│ Serializes:                          │
│ 1. RESPONSE_INDEX (uint32)   [4B]    │
│    = 41 (or any tracking ID)         │
│ 2. RESPONSE length (uint16)  [2B]    │
│ 3. RESPONSE content          [var]   │
│    = compilation output               │
│                                      │
│ Returns: std::vector<uint8_t>        │
│          total size = Y bytes        │
└──────────────────────────────────────┘
         │
         ▼
┌──────────────────────────────────────┐
│ Create Response WireHeader:          │
│                                      │
│ header.payload_length                │
│   = htonl(Y)                         │
│ header.msg_type                      │
│   = htons(0x0200)  // Server-to-Cli  │
└──────────────────────────────────────┘
         │
         ▼
┌──────────────────────────────────────┐
│ send_exact(newSocket, &header, 6):   │
│ Sends response header (6 bytes)      │
└──────────────────────────────────────┘
         │
         │ ┌─── 6 bytes over network ───┐
         │ │                            │
         ▼ ▼                            ▼
                                ┌──────────────────────────┐
                                │ recv_exact() from CLIENT │
                                │ Receives 6-byte header   │
                                │ msg_type = 0x0200        │
                                │ payload_len = Y          │
                                └──────────────────────────┘
                                           ▲
                                           │

┌──────────────────────────────────────┐
│ send_exact(newSocket, response, Y):  │
│ Sends response payload (Y bytes)     │
└──────────────────────────────────────┘
         │
         │ ┌─── Y bytes over network ───┐
         │ │  (fragmented by TCP OK)    │
         │ ▼                            ▼
         │                    ┌──────────────────────────┐
         │                    │ recv_exact() from CLIENT │
         │                    │ Receives Y-byte payload  │
         │                    │ Stores in buffer         │
         │                    └──────────────────────────┘
         │                              ▲
         ▼                              │

┌──────────────────────────────────────┐
│ Deserialize response payload:        │
│ (offset = 0)                         │
│                                      │
│ 1. Read RESPONSE_INDEX (4B)          │
│    memcpy() 4 bytes                  │
│    ntohl() convert                   │
│    offset += 4                       │
│                                      │
│ 2. Read RESPONSE length (2B)         │
│    memcpy() 2 bytes                  │
│    ntohs() convert                   │
│    offset += 2                       │
│                                      │
│ 3. Read RESPONSE content             │
│    memcpy() rsp_len bytes            │
│    RESPONSE.assign()                 │
└──────────────────────────────────────┘
         │
         ▼
┌──────────────────────────────────────┐
│ Print to GUI:                        │
│                                      │
│ terminalOutput->AppendText(          │
│   "Response from server:\n" +        │
│   RESPONSE                           │
│ )                                    │
│                                      │
│ User sees:                           │
│ "main.cpp:5: error: ..."             │
│ OR                                   │
│ "[compilation complete]"             │
└──────────────────────────────────────┘
         │
         ▼
┌──────────────────────────────────────┐
│ close(client_socket)                 │
│ WSACleanup() (Windows only)          │
└──────────────────────────────────────┘
         │
         ▼
┌──────────────────────────────────────┐
│ EXECUTION COMPLETE                   │
│                                      │
│ Status: Ready (for next command)     │
└──────────────────────────────────────┘
```

---

## 2. Wire Format Diagram

### **WireHeader Structure (6 bytes)**

```
Byte:  0        1        2        3        4        5
      ┌────────┬────────┬────────┬────────┬────────┬────────┐
      │            payload_length                 │ msg_type│
      │              (4 bytes, uint32)            │ (2 B)   │
      ├────────┬────────┬────────┬────────┬────────┬────────┤
      │  0x00  │  0x00  │  0x00  │  0x64  │  0x01  │  0x00  │
      └────────┴────────┴────────┴────────┴────────┴────────┘
                                  
      payload_length = 0x00000064 (100 bytes, network order)
      msg_type = 0x0100 (Client-to-Server, network order)
```

---

### **Client-to-Server Payload (Variable Length)**

```
Offset  Size    Field               Example Value
─────────────────────────────────────────────────────────
0       4       FILE_INDEX          0x0000002A (42)
4       2       FILE_NAME_LEN       0x0008 (8 bytes)
6       8       FILE_NAME           "main.cpp"
14      2       CMD_LEN             0x0017 (23 bytes)
16      23      COMPILE_COMMAND     "g++ main.cpp -o main"
39      4       FILE_CONTENT_LEN    0x000004E8 (1256 bytes)
43      1256    FILE_CONTENT        "#include <iostream>..."
─────────────────────────────────────────────────────────

Total Payload = 4 + 2 + 8 + 2 + 23 + 4 + 1256 = 1299 bytes
```

---

### **Server-to-Client Payload (Variable Length)**

```
Offset  Size    Field               Example Value
─────────────────────────────────────────────────────────
0       4       RESPONSE_INDEX      0x00000029 (41)
4       2       RESPONSE_LEN        0x0035 (53 bytes)
6       53      RESPONSE_CONTENT    "Compilation successful!\nExecutable created."
─────────────────────────────────────────────────────────

Total Payload = 4 + 2 + 53 = 59 bytes
```

---

## 3. TCP Transmission Breakdown

### **Example: Sending File "test.cpp" (50 bytes)**

```
Step 1: pack_file() generates payload
────────────────────────────────────────
Payload (65 bytes total):
  [4B] FILE_INDEX = 42
  [2B] FILE_NAME_LEN = 8
  [8B] FILE_NAME = "test.cpp"
  [2B] CMD_LEN = 20
  [20B] COMMAND = "g++ test.cpp -o test"
  [4B] FILE_LEN = 50
  [50B] FILE_CONTENT = [...actual C++ code...]

Step 2: Create WireHeader (6 bytes)
────────────────────────────────────────
  [4B] payload_length = htonl(65) = 0x00000041
  [2B] msg_type = htons(0x0100) = 0x0100

Step 3: send_exact(&header, 6)
────────────────────────────────────────
Network Activity:
  TCP Packet 1 → [0x00, 0x00, 0x00, 0x41, 0x01, 0x00]
  ✓ Sent: 6 bytes

Step 4: send_exact(&payload, 65)
────────────────────────────────────────
Network Activity (TCP may fragment):
  TCP Packet 2 → [0x00, 0x00, 0x00, 0x2A, 0x00, 0x08, ...]
                 [First 32 bytes of payload]
                 
  TCP Packet 3 → [... next 33 bytes of payload]
  
  ✓ Sent: 65 bytes (across multiple TCP packets)

Step 5: recv_exact() on server
────────────────────────────────────────
Server receives:
  - Packet 1: 6-byte header → knows to expect 65 bytes
  - Packets 2-3: 65 bytes of payload (TCP reassembles)
  
Server now has complete message ready to deserialize.
```

---

## 4. Error Handling Flow

```
CLIENT                                  SERVER
───────────────────────────────────────────────────────

connect() to server
    │
    ├─ Success: Continue
    │
    └─ Failure:
       • WSACleanup() (Windows)
       • close(socket) (Linux)
       • Print error to GUI
       • Return early
       ▼
     [Connection Failed]

send_exact() header
    │
    ├─ Success: Continue
    │
    └─ Failure (res <= 0):
       • Return false
       • Error printed
       • close(socket)
       ▼
     [Header Not Sent]

send_exact() payload
    │
    ├─ Success: Continue
    │
    └─ Failure:
       • Return false
       • close(socket)
       ▼
     [Payload Not Sent]

recv_exact() response header
    │
    ├─ Success: Continue
    │
    └─ Failure:
       • Print error
       • close(socket)
       ▼
     [Response Header Not Received]

payload_length > 64MB?
    │
    ├─ No: Continue
    │
    └─ Yes:
       • Print "Payload length exceed limit"
       • Return early
       • close(socket)
       ▼
     [Payload Too Large]

recv_exact() payload
    │
    ├─ Success: Continue
    │
    └─ Failure:
       • Print error
       • close(socket)
       ▼
     [Payload Not Received]

can_read() checks
    │
    ├─ All pass: Deserialize successfully
    │
    └─ Any fail:
       • Print "Unable to read [field]"
       • Return false
       ▼
     [Malformed Payload]
```

---

## 5. Message Type Reference

| Message Type | Value  | Direction       | Payload Contains |
|--------------|--------|-----------------|------------------|
| **File Submission** | 0x0100 | Client → Server | FILE_INDEX, FILE_NAME, COMMAND, FILE_CONTENT |
| **Response** | 0x0200 | Server → Client | RESPONSE_INDEX, RESPONSE_OUTPUT |

---

## 6. Byte Order Conversion Reference

```
htonl() - Host TO Network Long (uint32)
─────────────────────────────────────────
Little-endian host:    0x12345678 → 0x78563412 (reversed)
Network (big-endian):  0x78563412
Big-endian host:       0x12345678 → 0x12345678 (unchanged)

htons() - Host TO Network Short (uint16)
─────────────────────────────────────────
Little-endian host:    0x1234 → 0x3412 (reversed)
Network (big-endian):  0x3412
Big-endian host:       0x1234 → 0x1234 (unchanged)

ntohl() - Network TO Host Long
ntohs() - Network TO Host Short
─────────────────────────────────────────
(Reverse of htonl/htons - architecture independent)
```

---

## 7. Socket State Timeline

```
CLIENT                              SERVER
────────────────────────────────────────────────────

socket(AF_INET, SOCK_STREAM)       socket(AF_INET, SOCK_STREAM)
  │                                  │
  ▼                                  ▼
(TCP Client Socket)              (TCP Server Socket)
  │                                  │
  ├─ Set server_address             ├─ setsockopt(SO_REUSEADDR)
  │ inet_pton()                      │ (allow quick restart)
  │ htons(PORT)                      │
  │                                  │
  ▼                                  ▼
connect(server_addr)             bind(address, port)
  │                                  │
  ├─ Sends SYN packet    ────────➤  │
  │                                  ▼
  │                             listen(backlog=10)
  │                                  │
  │                                  ├─ Accept SYN, send SYN-ACK
  │                            ◀────┤
  │                                  │
  ├─ Sends ACK            ───────➤  │
  │                                  ▼
  ▼                            accept(blocking)
(Connected)                    (Returns newSocket)
  │                                  │
  ├─ Can send/recv        ◀───────➤ ├─ Can send/recv
  │ on client_socket                │ on newSocket
  │                                  │
  ├─ send_exact(header)   ───────➤ recv_exact(header)
  │ send_exact(payload)   ───────➤ recv_exact(payload)
  │                                  │
  │                            (process, compile)
  │                                  │
  ├─ recv_exact(header)   ◀────────┤ send_exact(header)
  │ recv_exact(payload)   ◀────────┤ send_exact(payload)
  │                                  │
  ▼                                  ▼
close(client_socket)          close(newSocket)
                              close(hostSocket)
```

---

## 8. Key Concepts Summary

| Concept | Purpose | Example |
|---------|---------|---------|
| **WireHeader** | Frame delimiter, tells receiver payload size | `{ len: 100, type: 0x0100 }` |
| **htonl/ntohs** | Endianness conversion for network compatibility | x86 ↔ ARM safe transmission |
| **send_exact** | Guarantees all bytes sent (TCP may fragment) | Loop until sent == total |
| **recv_exact** | Guarantees all bytes received | Loop until rx == total |
| **pack_file** | Serializes structured data into byte stream | `[INDEX][LEN][NAME]...` |
| **Deserialize** | Reconstructs structured data from bytes | Reverse of packing |
| **popen/pclose** | Execute command and capture output | `popen("g++ ...", "r")` |


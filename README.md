
# RCE - Remote Code Execution

This project is a Remote Code Execution framework designed to securely send and execute code on a remote server, returning the execution output to the client machine over a network connection. The primary goal is to offload the computational load to the server machine and then transmit the output back to the client machine.
> **Warning:** By design, this application allows arbitrary code execution on the server host. It is strictly intended for controlled environments, private networks, or authorized remote task execution. Proper network access controls (like firewalls or VPNs) must be implemented to prevent unauthorized access.

## Usage

### Server Side
This mode requires specifying a **port** and the **command to execute**. The command runs on the server machine, and the stdout/stderr output is sent back to the client."
### Client Side
This mode requires specifying the **port**, **target IP**, and the **local file path** to compile or execute on the server.
>***Caution:*** Windows users must provide an absolute file path.

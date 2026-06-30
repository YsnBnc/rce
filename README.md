
# RCE - Remote Code Execution

RCE provides code execution in another machine then return the output to the original machine via network. Ideally the goal is executing resource-heavy codes in powerful machine where sent from relatively weak machine. Ultimate goal is making this project widely avaliable for all programming languages and securely transfering data between machines.

## Installation

1 - Clone the project.

```bash
  git clone https://github.com/YsnBnc/rce.git 
```
2 - Build it with CMake.

## Usage

After build. Run the program and select a side. After the application the output will send to the client.
### Server Side
This selection will require **port** and **command to execute**. This command will be executed server machine and will return the output to the client.
### Client Side
This selection will require **port**,**Target-IP** and **File Path** to compile selected file on the server.
>***Warning:*** For Windows, file path should be absolute path.

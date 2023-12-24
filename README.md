# Multiplayer Pong Installation Guide

## Getting Started

### Setting Up for Multiplayer

1. **Server Setup:**
   - The server address is set to a loop-back address by default.
   - To play with others, the server host needs to find their IPv4 address.
   - Open Command Prompt and type `ipconfig`.
   - Look for the 'IPv4 Address' under your active network connection.
   - Share this IPv4 address with the client players.

2. **Client Setup:**
   - The client needs to update the server IP address in the source code.
   - Edit the file `Client.cpp` in the project.
   - Change the IP address at [line 22](https://github.com/antoniukoff/Multithreaded-Pong/blob/main/Pong/Client.cpp#L22) to the IPv4 address provided by the server host.
   - Additionally, update the address at [line 37](https://github.com/antoniukoff/Multithreaded-Pong/blob/main/Pong/Client.cpp#L37) if necessary.
   - Make sure the port number in the client matches the server's port.
   - Compile and run the updated client to connect to the server.

### Playing the Game

- Launch 'Multiplayer Pong' from the executable file.
- Once the application is running, you will be presented with two options:
  - Press `1` to host.
  - Press `2` to join.
- Follow on-screen instructions to start playing.

## Additional Notes

- This project comes with all necessary libraries pre-installed.

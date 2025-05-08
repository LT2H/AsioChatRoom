# Asio ImGui Chat Room Application (Client/Server)
A client-server chat room application using a small custom framework to abstract **Asio** for networking and **ImGui** for the GUI.

![image](https://github.com/user-attachments/assets/2fc5ca88-d87c-4123-a18c-86bde2735795)

## Features
- **Real-time messaging**: Multiple clients can send and receive messages instantly through the server.
- **Chat window with input and output areas**: The user interface includes a text area to display chat messages and an input box to type messages.
- **Message history**: The chat window displays the history of messages.
- **Client validation**: Validates clients before they connect to the server.
- **Track online clients**: Server and clients can see the list of connected users.

## Technical Details
  - **Language**: C++
  - **Standard**: C++20
  - **Dependencies**: Asio(standalone), ImGui
  - **Build System**: CMake
  - **Platform**: Windows, Linux
  - **Socket Communication**: TCP/IP using Asio for network communication

## Getting Started

1. Clone this repository:
    ```sh
    git clone https://github.com/LT2H/AsioChatRoom.git
    
2. Build Server and Client
    ```sh
    cd AsioChatRoom
    cmake -S . -B build
    cmake --build build
    
3. Run the Server then Client(s) inside `build/apps/Debug`.

## License
- This repository is licensed under Apache 2.0 (see included LICENSE.txt file for more details)

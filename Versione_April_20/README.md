*This project has been created as part of the 42 curriculum by cacorrea, sel-khao, vloddo.*

# ft_irc

## Description
**ft_irc** is a minimal implementation of an IRC (Internet Relay Chat) server written in C++.  
The goal of this project is to reproduce the core behavior of an IRC server compliant with the protocol, allowing multiple clients to connect, communicate, and interact in real time.

The server handles multiple simultaneous connections using non-blocking I/O and supports essential IRC features such as:
- User authentication
- Nickname and username management
- Channel creation and management
- Private and group messaging

This project focuses on key concepts such as network programming, socket management, and protocol implementation.

## Instructions
The server supports the core functionalities required by standard IRC clients:

- **Authentication & Connection**: password-protected server access and handling of the `PASS`, `NICK`, and `USER` commands for client registration.
- **Channel Management**: users can create, join, and leave channels using `JOIN`, `PART`, and `QUIT`.
- **Messaging**: private messages between users and broadcast messages within channels using `PRIVMSG` and `NOTICE`.
- **Channel Operators**: certain users can be granted operator privileges to manage channel operations.
- **Operator Commands**:
  - `KICK`: remove a client from a channel
  - `INVITE`: invite a client to a channel
  - `TOPIC`: set or view the channel topic
  - `MODE`: modify channel modes (e.g., invite-only mode, password protection, user limit, operator privileges)


### Compilation
To compile the program, use: **make**

### Execution
To start the Server, use: ./ircserv <port> <password>
 - `port`: The port number on which the IRC server listens for incoming connections.
 - `password`: the password required for client authentication. 

To connect to the server, you can use: nc <IP ADDRESS> <PORT>
 - `IP ADDRESS`: the host IP address.
 - `PORT`: the port used by the server.

You can also use an IRC Client such as HexChat, WeeChat...

## Resources

- IRC protocol documentation (https://datatracker.ietf.org/doc/html/rfc1459) was studied to understand the protocol specifications and to guide the implementation of its behavior and message handling.
- Beej's Guide to Network Programming (https://beej.us/guide/bgnet/) was used as a reference for socket programming and networking fundamentals.
- AI tools (ChatGPT, DeepSeek) were used to clarify concepts, provide explanations, and suggest ways to organize and divide the work.
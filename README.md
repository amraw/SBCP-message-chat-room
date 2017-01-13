# SBCP-message-chat-room
Developed a Chat Server using Simple Broadcast Chat Protocol (SBCP) that allowed Clients to join and leave a chat session, view members of the session and send and receive messages. This was implemented using socket programming in C in Linux terminal.

ECEN602 Programming Assignment 01
----------------------------------------------
Team Number:23
Member 1: Kranthi Kumar Katam  (UIN: 225009204)
Member 2: Matthew Roe          (UIN: 321007055)
Member 3: Jifang Mu            (UIN: )
-----------------------------------------------
In this project assignment, we built a simple chat room for the client and server. The server provides a chat room and the client joins the chat room. The server only can handle finite number of clients and a client will receive a list of connected members of the chat session once they complete the join transaction. The client sends the Join request and the server accept it, the client send the message and the server forward a message to the client. The client can exit the session anytime. In this programming assignment, we will use C programming language and test the result in a Linux environment. 
----------------------------------------------
The package contains 3 files:
1.server.c
2.client.c
3.makefile
-----------------------------------------------
Functions implemented:
JOIN
SEND
FWD
-----------------------------------------------
server.c
1) Server creates a socket and binds it to the IP address and Port number. The maximum no. of clients is set by the user.
2) We made three structures (msg_header, msg_attribute and message) to construct the message in SBCP format.
3) After binding the socket, the server will keep on listening for any of the clients to connect using the listen call function.
4) Server uses select call function to select the client which is trying to connect.
5) If server recieves a new connection it will set readfd to client socket discriptor and if the client disconnects from server, the server clears the readfd and closes the socket.
6) When a join request comes to the server, the server will accept the client.
7) The server will recieve the client's message through recv() call. And it sends the information to other clients by send_all() call. 

-----------------------------------------------
client.c

1) The client uses joinServer( ) to send the join request to the server.
2) The client uses connect( ) to connect the server
3) The client uses receiveMessage( ) to receive the message from the server. After successfully connected with the server, the server returns ACK with the a single client count and username of clients.
4) The client uses sendMessage( ) to send a message to the server.
5) The client will receive a list of connected members of the chat session once he completes the join transaction.
6) The client can detect idle clients and sends the message to the server indicating that he has been idle.

-----------------------------------------------

To compile the code, run the makefile: make -f make_serv_client
Run the server by using the command line: ./server
Run the client by using the command line: ./client

# Socket_Communcation

Name: Muhao Guo
USC ID: 3441981541

What you have done in the assignment?
  Realized the socket communication between mainserver and serverA and serverB and clients (client1, client2).
  There are 5 cpp files, they are serverA.cpp, serverB.cpp, mainserver.cpp, client1.cpp, client2.cpp. One Makefile file and one readme file.
  When input a user id and country name in client, the client will get a recommended friend's id if it has.

What your code files are and what each one of them does?
  1. serverA.cpp / serverB.cpp:
    (1) getdata from data1.txt(or data2.txt)
    (2) bootup and initialize the UDP socket
    (3) receive command from the mainserver and knows that mainserver is startup
    (4) get the information of countries from data1.txt(or data2.txt), and send the information to mainserver
    (5) receive command from mainserver and get the id and country
    (6) run recommendation algorithm and reply the recommended results to mainserver  
  2. mainserver.cpp:
    (1) bootup and initialize the CTP socket and UDP socket
    (2) send command to serveA and serverB and let them know mainserver has started up
    (3) receive the information of countries from serverA and serverB
    (4) print a country table
    (5) wait and receive the command from clients
    (6) after get command(id and country) from a client,judge the country is belong to serverA or serverB and send this command to serverA or serverB
    (7) receive the recommended results from serverA or serverB
    (8) send the recommended results to client
  3.client1.cpp / client2.cpp:
    (1) bootup and initialize the TCP socket
    (2) get the command(id and country) form keyboard
    (3) send command to mainserver
    (4) get recommended result
  
  
The format of all the messages exchanged
  Using string format for messages exchange.

Reused Code:
  When I initialize the UDP socket and TCP socket, I refer the book: Beej’s Guide to Network Programming Using Internet Sockets.
  
How to run:
  Before run the program, it need us add a data1.txt and data2.txt in the same root.
  1.make all 
  2.make serverA
  3.make serverB
  4.make mainserver
  5./client1
  6./client2

Environment and Language:
  Ubuntu (16.04), C++

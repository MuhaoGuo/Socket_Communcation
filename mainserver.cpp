#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <iostream>
#include <sstream>
#include <map>
#include <iomanip>

using namespace std;

#define LOCAL_HOST "127.0.0.1"
#define B_UDP_PORT "31541"
#define A_UDP_PORT "30541"
#define MAIN_SERVER_UDP_PORT "32541"
#define MAIN_SERVER_TCP_PORT "33541"

struct addrinfo hints;
struct addrinfo *res_A_server;          // getaddrinfo A' s result
struct addrinfo *res_B_server;          // getaddrinfo B' s result
struct addrinfo *res_backend_server;    // getaddrinfo backend server' s result

struct addrinfo *res_main_server_UDP;   //  main_server_UDP 's info
struct addrinfo *res_main_server_TCP;   //  main_server_TCP 's info

struct addrinfo *res_client_TCP;        //  client‘s info, TCP

struct sockaddr_in child_TCP_info;       // special structure, use in accept   ????? 

int UDP_SOCKET_STATUS;   // UDP socket 
int TCP_SOCKET_STATUS;   // TCP socket
int child_TCP_STATUS;    // 

#define MAXDATASIZE 1024
char buf[MAXDATASIZE];     // buffer, receive data will use it

map<string, string> country_list;
istringstream ss;

socklen_t addr_size;  // special structure, use in accept

void init_UDP();
void init_TCP();
void sendto_data(string msg, struct addrinfo *res_info);
void receivefrom_data(struct addrinfo *res_info);
void send_data(string msg);
void receive_data();
void print_table();
void process_country_list();


void init_UDP(){
    //(1)add the struct addrinfo by hand 
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;     // set to AF_INET to force IPv4   （Af: address family），
	hints.ai_socktype = SOCK_DGRAM;  //UDP

	// (2)get A server's info
	int rv_A_status;   //  status of the getaddrinfo
	if(rv_A_status = getaddrinfo(LOCAL_HOST, A_UDP_PORT, &hints, &res_A_server)!=0){
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv_A_status));
		return;
	}

    // (3)get B server's info
	int rv_B_status;   //  status of the getaddrinfo
	if(rv_B_status = getaddrinfo(LOCAL_HOST, B_UDP_PORT, &hints, &res_B_server)!=0){
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv_B_status));
		return;
	}

    // (4)get backend server's info
	int rv_backend_status;   //  status of the getaddrinfo
	if(rv_backend_status = getaddrinfo(LOCAL_HOST, NULL, &hints, &res_backend_server)!=0){
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv_backend_status));
		return;
	}

	//(5) after we get the backend server's info , we create a SOCKET 
    UDP_SOCKET_STATUS = socket(res_backend_server->ai_family, res_backend_server->ai_socktype, res_backend_server->ai_protocol);
	if(UDP_SOCKET_STATUS==-1){   // if there is error to create a socket
		perror("ERROR: Cannot open backend server socket");
		close(UDP_SOCKET_STATUS);
		exit(1);
	}
    // main server's info, UDP 
    getaddrinfo(LOCAL_HOST, MAIN_SERVER_UDP_PORT, &hints, &res_main_server_UDP);

    //bind the SOCKET with the MAIN_SERVER_UDP_PORT  "32541"
    bind(UDP_SOCKET_STATUS, res_main_server_UDP->ai_addr, res_main_server_UDP->ai_addrlen);
}

void init_TCP(){
    memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
    // get the info of the client, TCP
	getaddrinfo(LOCAL_HOST, NULL, &hints, &res_client_TCP);

    // create socket:
    TCP_SOCKET_STATUS = socket(res_client_TCP->ai_family, res_client_TCP->ai_socktype, res_client_TCP->ai_protocol);
	if(TCP_SOCKET_STATUS == -1) {
		perror("ERROR: Cannot open client socket");
		close(TCP_SOCKET_STATUS);
		exit(1);
	}
    // get the info of the main server, TCP
    getaddrinfo(LOCAL_HOST, MAIN_SERVER_TCP_PORT, &hints, &res_main_server_TCP);

    // bind the socket with the main server TCP port
	int bind_status;
    bind_status = bind(TCP_SOCKET_STATUS, res_main_server_TCP->ai_addr, res_main_server_TCP->ai_addrlen);
	if(bind_status == -1){
		perror("ERROR: bind TCP socket!!");
		close(TCP_SOCKET_STATUS);
		exit(1);
	}

    // listen. in  main server TCP port
    #define BACKLOG 10  // the number of connections allowed on the incoming queue.
	int listen_status;
    listen_status = listen(TCP_SOCKET_STATUS, BACKLOG);
	if(listen_status == -1){
		perror("ERROR: listen from client!!");
		close(TCP_SOCKET_STATUS);
		exit(1);
	}
}

void sendto_data(string msg, struct addrinfo *res_info){
    int len = msg.length();
	char req[len];
	strcpy(req, msg.c_str()); // string to char* 
    if(sendto(UDP_SOCKET_STATUS, req, len, 0, res_info->ai_addr, res_info->ai_addrlen) == -1){
    	perror("ERROR: Fail to send data to backend server");
		close(UDP_SOCKET_STATUS);
		exit(1);
    }
    // cout <<  "sendto_data fnction finish" << endl;
}

void recevefrom_data (struct addrinfo *res_info){
    // int numbytes;
	memset(buf, 0, sizeof buf);  	    //empty buffer
	socklen_t addr_len;                 //socklen_t is a special data structure
	addr_len = sizeof res_info;         //get the size 
	if(recvfrom(UDP_SOCKET_STATUS, buf, MAXDATASIZE , 0, res_info->ai_addr, &addr_len) ==-1){
		perror("ERROR: Fail to receive reply from server A/B");
		close(UDP_SOCKET_STATUS);
		exit(1);
	}
}

void send_data(string msg){
    char reply[msg.length()];
    strcpy(reply, msg.c_str()); //  string to char* 
	if(send(child_TCP_STATUS, reply, msg.length(), 0) == -1) {
		perror("ERROR: Fail to send reply to client");
		close(child_TCP_STATUS);
		exit(1);
	}
}

void receive_data(){
	// int numbytes;
	memset(buf, 0, sizeof buf);  	    //empty buffer
	if (recv(child_TCP_STATUS, buf, MAXDATASIZE , 0) == -1){
		perror("ERROR: Fail to receive reply from client");
        close(child_TCP_STATUS);
		exit(1);
	}
}

void print_table(){
	string t1 = "";
	string t2 = "";
	for(map<string,string>::iterator it=country_list.begin(); it != country_list.end(); it++) {
		if(it->second == "ServerA"){
			t1 += it->first + " ";
		}else if(it->second == "ServerB"){
			t2 += it->first + " ";
		}
	}

	istringstream s1(t1);
	istringstream s2(t2);
	cout << left << setfill(' ') << setw(20) << "Server A" << "|Server B" << endl;
	string w1, w2;
	while((!s1.eof() || !s2.eof())) {
		s1 >> w1;
		s2 >> w2;
		if(s1.eof())
			w1 = " ";
		if(s2.eof())
			w2 = " ";
		if(w1 != " " || w2 != " ")
			cout << left << setfill(' ') << setw(20) << w1 << "|" << w2 << endl;
	}
}

void process_country_list(string server) {
	string backend_server_reply = buf;

	ss.clear();
	ss.str(backend_server_reply);

	string word;

 	while(ss >> word) {  
		country_list.insert(pair<string, string>(word, server));
	}
}


int main(){ 
    init_TCP();
    init_UDP();
    cout << "The Main server is up and running." << endl;
    
    string req ="give mian server the list";
    // request A for the country list
    sendto_data(req, res_A_server);
    recevefrom_data(res_A_server);
	cout << "The Main server has received the country list from server A using UDP over port " <<  MAIN_SERVER_UDP_PORT << endl;
    string serverA = "ServerA";
	process_country_list(serverA);

    // request B for the country list   
    sendto_data(req, res_B_server);
    // int res_from_B;
    recevefrom_data(res_B_server);  
    // if(res_from_B != -1){    // receive the relpy from B 
    cout << "The Main server has received the country list from server B using UDP over port " <<  MAIN_SERVER_UDP_PORT << endl;
    // }
	string serverB = "ServerB";
    process_country_list(serverB);

    print_table();

    while(1){
        addr_size = sizeof child_TCP_info;
		if((child_TCP_STATUS = accept(TCP_SOCKET_STATUS, (struct sockaddr *) &child_TCP_info, &addr_size)) == -1) {
			perror("ERROR: Fail to accept TCP connection");
			close(TCP_SOCKET_STATUS);
			exit(1);
		}
        if(fork() == 0){ // child
            while(1){
                receive_data();
                string country;
                string id;
                ss.clear();
				ss.str(buf);   
				ss >> country; 
				ss >> id; 
				// cout << " ----------------------------"  << endl;
				// cout << "id is: " <<  id << endl;
				// cout << "country is : " <<  country << endl;  
				// cout << " ----------------------------"  << endl;
				///////////////   ??????????? client id ???? 
                cout << "The Main server has received the request on User "<< id << " in " << country << " from the client using TCP over port "<< MAIN_SERVER_TCP_PORT << endl;
                map<string, string>::iterator it = country_list.find(country);
                
                if(it != country_list.end()){    // find this country in country list
                    string server = it->second;
                    cout << country << " shows up in server " << server << endl;
                    if(server == "ServerA") {                            // in server A; 
						sendto_data(country + " " + id, res_A_server);   // send to A 
						cout << "The Main Server has sent request from User " << id << " to serverA using UDP over port " <<  MAIN_SERVER_UDP_PORT <<endl;
						recevefrom_data (res_A_server);                  // get reply from A
					} else if(server == "ServerB"){                      // in server B;
						sendto_data(country + " " + id, res_B_server);   //  send to B 
						cout << "The Main Server has sent request from User " << id << " to serverB using UDP over port " <<  MAIN_SERVER_UDP_PORT <<endl;
						recevefrom_data (res_B_server);                  // get reply from B 
					}

                    ss.clear();
                    ss.str(buf);   
                    string command, backend_server;
                    ss >> command;
                    ss >> backend_server;

					// cout << "----------------------------------------------" << endl;
					// cout << "command from backserver is " << command << endl;
					// cout << "backend server from backserver is " << backend_server << endl;
					// cout << "----------------------------------------------" << endl;
					
                    if(command == "NO_ID") { // if no user id found
                        cout << "The Main server has received \"User ID: Not Found\" from server " << backend_server;
                        send_data("NO_ID " + country + " " + id);     // error message : cannot find the id
                        cout << "The Main Server has sent error to client using TCP over " << MAIN_SERVER_TCP_PORT << endl;
                    }else{                 // if the id find and receive the result from backend server
						string recommended_user;
						ss >> recommended_user;
                        // we should send the result to the client
                        cout << "The Main server has received searching result(s) of User " << id << " from server " << backend_server << endl;
                        send_data("FIND "  + country + " " + id + " " + recommended_user);
                        cout << "The Main Server has sent seraching result(s) to client using TCP over port " << MAIN_SERVER_TCP_PORT << endl;
                    }
                }else{   // cannot find that country
                    cout<< country << " does not show up in server A&B" << endl;
					send_data("NO_COUNTRY " + country + " " + id);
					// clientd id 
					cout << "The Main Server has sent \"Country Name: Not Found\" to the client1/2 using TCP over port "  << MAIN_SERVER_TCP_PORT << endl;
                }
            }
        }
    }
}

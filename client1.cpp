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


using namespace std;


#define LOCAL_HOST "127.0.0.1"
#define MAIN_SERVER_TCP_PORT "33541"

#define MAX_DATA_SIZE 1024
char buf[MAX_DATA_SIZE];

struct addrinfo hints;
struct addrinfo *res_main_server;
int TCP_SOCKET_status;

istringstream ss;

string input_country;
string input_id;



void init_TCP();
void send_query();
void receive_data(string country, string id);
void receive_result();

// create socket and connect to server
void init_TCP(){
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

    // get addrinfo
    getaddrinfo(LOCAL_HOST, MAIN_SERVER_TCP_PORT, &hints, &res_main_server);

    // create socket
    TCP_SOCKET_status = socket(res_main_server->ai_family, res_main_server->ai_socktype, res_main_server->ai_protocol);
    if(TCP_SOCKET_status == -1) {
		perror("ERROR: Cannot open client socket");
		exit(1);
	}

    //connect to main server.
    int connect_status;
    connect_status = connect(TCP_SOCKET_status, res_main_server->ai_addr, res_main_server->ai_addrlen);
    if(connect_status == -1){
        perror("ERROR: Cannot connect to main server");
		close(TCP_SOCKET_status);
		exit(1);
    }
    // client id?????
    cout << "Client1 is up and running" << endl;
}

void send_query (string country, string id){
    string msg = country + " " + id;
    // convert string to char* format
    char * strc = new char[strlen(msg.c_str())+1];
    strcpy(strc, msg.c_str());
 
    send(TCP_SOCKET_status, strc , msg.size(), 0); // the second parameter: addr will usually be a pointer to a local struct sockaddr_storage.
    // client id ??? 
    cout << "Client1 has sent User " << id  << " and " << country << " to Main Server using TCP" << endl;
}

void receive_result(){
    string command, country, id;
    memset(buf, 0, sizeof buf);
    recv(TCP_SOCKET_status, buf, MAX_DATA_SIZE, 0);

    string rec_msg(buf);

    ss.clear();
    ss.str(buf);
    ss >> command;
	ss >> country;
	ss >> id;

    if (command == "NO_COUNTRY"){        // NO_COUNTRY
        cout << country <<  " not found" << endl;
    }else if(command == "NO_ID"){        // NO_ID
        cout << "User " << id <<  " not found" << endl;
    }else {                              // get the result
        string user;
		string all_recommand_users = "";
		while(ss >> user) {
		    all_recommand_users += "User" + user + ", ";
		}
        // client id ????????
		cout << "Client1 has results from Main Server :" << all_recommand_users << "is/are possible friend(s) of User " << id << " in " << country << endl;
    }

}


int main(){
    init_TCP();
    while(1){
        // get the country and id from the keyboard
        cout << "Please enter the User ID: ";
		getline(cin, input_id);

        cout << "Please enter the Country Name: " ;
        getline(cin, input_country);  

        // send the data:(query)
        send_query(input_country, input_id);

        // get the result from the main server:
        receive_result();

        cout << "----------Start a new request-----------" << endl;
    }
    return 0;
}





// int main(){
    // //创建套接字
    // int sock = socket(AF_INET, SOCK_STREAM, 0);
    // //向服务器（特定的IP和端口）发起请求
    // struct sockaddr_in serv_addr;
    // memset(&serv_addr, 0, sizeof(serv_addr));  //每个字节都用0填充
    // serv_addr.sin_family = AF_INET;  //使用IPv4地址
    // serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");  //具体的IP地址
    // serv_addr.sin_port = htons(1234);  //端口
    // connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
   
    // //读取服务器传回的数据
    // char buffer[40];
    // read(sock, buffer, sizeof(buffer)-1);
   
    // printf("Message form server: %s\n", buffer);
   
    // //关闭套接字
    // close(sock);
    // return 0;
// }
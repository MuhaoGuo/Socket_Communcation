#include<fstream>
#include<iostream>
#include <sstream>
#include <map>
#include <set>
#include <string.h>
#include <ctype.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>


using namespace std;

#define LOCAL_HOST "127.0.0.1"
#define B_UDP_PORT "31541"
#define MAIN_SERVER_UDP_PORT "32541"
#define MAIN_SERVER_TCP_PORT "33541"


#define MAXDATASIZE 1024  //  this size is used for buffer 
char buf[MAXDATASIZE];  // define buffer size
int UDP_SOCKET_STATUS;   // this is the UDP socket we will create 

string all_countrise_names; // this is the string of all countries name split by " " B will send 

map<string, map<string, set<string> > > all_data;   // key: country_name, value: the data
struct addrinfo hints;
struct addrinfo *res_main_server;
struct addrinfo *res_B_server;


void getdata();
void UDP_init();
void send_data();
void receive_data();

void getdata(){
	ifstream data2("data2.txt"); 
	string line;
	int num_country = 0;
	string country_name;
	map<string, set<string> > country_i;   // store every country's info

	while (getline (data2, line)){   // output by line
		istringstream ss(line);      //  get every line
		string s;
		ss >> s;                     // first word of the line		

		char c = s.at(0);
		if(!(c >='0' && c<='9')){            // if it is not number
			if(num_country > 0){             // the first country, we do not add something
				all_data.insert(pair<string, map<string, set<string> > >(country_name, country_i));  // add this country's info.
				country_i.clear();           // clear the map
			}
			country_name = s;
			all_countrise_names += s + " ";
			// cout << all_countrise_names << endl;
			num_country ++;
		}else{
			string id = s;          // get id
			set<string> neighbors;  // get id's neighbors
			while(ss >> s){
				// cout << s << endl;
				neighbors.insert(s);
			}
			country_i.insert(pair<string, set<string> >(id, neighbors));   // add this id's info
		}
	}
	all_data.insert(pair<string, map<string, set<string> > >(country_name, country_i));   // add the last country's info
}

void UDP_init(){
	//(1)add the struct addrinfo by hand 
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4   （Af: address family），
	hints.ai_socktype = SOCK_DGRAM;  //UDP
	// hints.ai_flags = AI_PASSIVE; // use my IP

	//(2)get main server's info
	getaddrinfo(LOCAL_HOST, MAIN_SERVER_UDP_PORT, &hints, &res_main_server);

	//(3) after we get the main server's info , we use it to create a SOCKET 
	UDP_SOCKET_STATUS = socket(res_main_server->ai_family, res_main_server->ai_socktype, res_main_server->ai_protocol);
	if(UDP_SOCKET_STATUS==-1){   // if there is error to create a socket
		perror("ERROR: server B cannot create socket");
		close(UDP_SOCKET_STATUS);
		exit(1);
	}

	// reset the struct addrinfo by hand
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	//(4)get B server's info
	int rv_B;
	// int status_B;
	if(rv_B = getaddrinfo(LOCAL_HOST, B_UDP_PORT, &hints, &res_B_server)!=0){
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv_B));
		return;
	}

	//(5) bind: because in B, we want to listen,  so we bind a port of B machine with socket
	if(bind(UDP_SOCKET_STATUS, res_B_server->ai_addr, res_B_server->ai_addrlen) == -1) {
		perror("ERROR: Fail to bind UDP socket");
		close(UDP_SOCKET_STATUS);
		exit(1);
	}
	cout << "The server B is up and running using UDP on port is " << B_UDP_PORT << endl;
}

// receive data in UDP:  recvfrom
void receive_data(){
	// int numbytes;
	memset(buf, 0, sizeof buf);  	    //empty buffer
	socklen_t addr_len;                 //socklen_t is a special data structure
	addr_len = sizeof res_main_server;  //get the size 
	if (recvfrom(UDP_SOCKET_STATUS, buf, MAXDATASIZE-1 , 0, res_main_server->ai_addr, &addr_len) == -1){
		perror("recvfrom");
		// exit(1);
	}
}

// send data in UDP : sendto
void send_data(string data){
	char counrties_info[data.length()];
	strcpy(counrties_info, data.c_str());   // copy from data to counrties_info
	cout<< "---------------------" << endl;
	cout << "send from B to Main is " << counrties_info << endl;
	cout<< "---------------------" << endl;
	if((sendto(UDP_SOCKET_STATUS, counrties_info, data.length(), 0, res_main_server->ai_addr, res_main_server->ai_addrlen)) == -1){	
		perror("talker: sendto");
		close(UDP_SOCKET_STATUS);
		exit(1);
	}
	// cout << "The server A has sent a country list to Main Server ？？？？？" << endl;
}




int main (){
	getdata();      // get data from test1.txt
	UDP_init();     // initial the UDP
	receive_data(); //  receive the command from the main_server
	send_data(all_countrise_names);  // send data of all countries to main server
	cout << "The server B has sent a country list to Main Server" << endl;

	while(true){
		string country_name, id;
		receive_data();
		istringstream ss(buf);

		ss >> country_name;
		ss >> id;
		if(!country_name.empty() && !id.empty()){  // if string  not empty
			cout << "The server B has received request for finding possible friends of User "<< id << "in "<< country_name << endl;
		}

		map<string, map<string, set<string> > >::iterator it = all_data.find(country_name);   // find the country_name if in the map 

		if(it != all_data.end()){ //  the iterator does not get to end, so there has such country name
			map<string, set<string> > country_map = it->second;
			map<string, set<string> >::iterator sub_it = country_map.find(id);  // sub iterator , find the id
			
			if(sub_it != country_map.end()) {    // if we find this id 
				cout<< "The server B is searching possible friends for User " << id << "...." << endl;
				set<string> id_nbr_set = sub_it->second;   // this is this id' s neighbor set
				set<string>::iterator sub_sub_iter;

				string neighbor_users = "";   //////--------??????????????  we will print all this id's neighbors
				for(sub_sub_iter = id_nbr_set.begin(); sub_sub_iter != id_nbr_set.end(); sub_sub_iter++) {
					neighbor_users += *sub_sub_iter;
					neighbor_users += " ";
				}

				cout << "Here are the results:" << neighbor_users << endl;     ///// ????  需计算出结果

				send_data("FIND ServerB" + neighbor_users);  ////   ------------- ??????? 
				cout << "The server B has sent the result(s) to Main Server" << endl;
			} else {
				send_data("NO_ID");
			}
		}else{
			send_data("NO_COUNTRY");
		}
	}
}
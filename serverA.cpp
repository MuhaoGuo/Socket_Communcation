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
#define A_UDP_PORT "30541"
#define MAIN_SERVER_UDP_PORT "32541"
#define MAIN_SERVER_TCP_PORT "33541"


#define MAXDATASIZE 1024  //  this size is used for buffer 
char buf[MAXDATASIZE];  // define buffer size
int UDP_SOCKET_STATUS;   // this is the UDP socket we will create 

string all_countries_names; // this is the string of all countries name split by " " A will send 

map<string, map<string, set<string> > > all_data;   // key: country_name, value: the data
struct addrinfo hints;
struct addrinfo *res_main_server;
struct addrinfo *res_A_server;


void getdata();
void UDP_init();
void send_data(string data);
void receive_data();
void recommendation_algorithm(map<string, set<string> > country_map, string id);

void getdata(){
	ifstream data1("data1.txt"); 
	string line;
	int num_country = 0;
	string country_name;
	map<string, set<string> > country_i;   // store every country's info

	while (getline (data1, line)){   // output by line
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
			all_countries_names += s + " ";
			// cout << all_countries_names << endl;
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
		perror("ERROR: server A cannot create socket");
		close(UDP_SOCKET_STATUS);
		exit(1);
	}

	// reset the struct addrinfo by hand
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	//(4)get A server's info
	int rv_A;
	// int status_A;
	if(rv_A = getaddrinfo(LOCAL_HOST, A_UDP_PORT, &hints, &res_A_server)!=0){
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv_A));
		return;
	}

	//(5) bind: because in A, we want to listen,  so we bind a port of A machine with socket
	if(bind(UDP_SOCKET_STATUS, res_A_server->ai_addr, res_A_server->ai_addrlen) == -1) {
		perror("ERROR: Fail to bind UDP socket");
		close(UDP_SOCKET_STATUS);
		exit(1);
	}
	cout << "The server A is up and running using UDP on port is " << A_UDP_PORT << endl;
}

// receive data in UDP:  recvfrom
void receive_data(){
	// int numbytes;
	memset(buf, 0, sizeof buf);  	    //empty buffer
	socklen_t addr_len;                 //socklen_t is a special data structure
	addr_len = sizeof res_main_server;  //get the size 
	if(recvfrom(UDP_SOCKET_STATUS, buf, MAXDATASIZE-1 , 0, res_main_server->ai_addr, &addr_len) == -1){
		perror("recvfrom");
		// exit(1);
	}
}

// send data in UDP : sendto
void send_data(string data){
	char counrties_info[data.length()];
	strcpy(counrties_info, data.c_str());   // copy from data to counrties_info
	// cout<< "---------------------" << endl;
	// cout << "send from A to Main is " << counrties_info << endl;
	// cout<< "---------------------" << endl;
	if((sendto(UDP_SOCKET_STATUS, counrties_info, data.length(), 0, res_main_server->ai_addr, res_main_server->ai_addrlen)) == -1){	
		perror("talker: sendto");
		close(UDP_SOCKET_STATUS);
		exit(1);
	}
	// cout << "The server A has sent a country list to Main Server ？？？？？" << endl;
}


//given u (id) and all_data of this country. 
//1. find the set N :  n has no connected neighbors with u
//2. find the set M :  m has connected neighors with u
//3. if N is not empty:  for every n in N : count the common neighbors of u(id): n's neighbor is also u's neighbor? yes -> +1  , for each n, we have a same_nbr_n
							// if same_nbr_n == 0 ? -> highest degree: n' which has the most neighbors --> smallest id
							// else :same_nbr_n > 0  ? --> the bigest same_nbr_n related n is the n,  --> smallest id 
	// else: N is empty,   None 

// input:  
// 		map<string, set<string> > country_map
// 		string id
#include <algorithm>
string recommended_id;
void recommendation_algorithm(map<string, set<string> > country_map, string id){
	set<string> all_users;

	// traverse the country_map , get all users in this country 
	map<string, set<string> >::iterator iter;
    iter = country_map.begin();
    while(iter != country_map.end()) {
		set<string> user_neighbors = iter->second;
		all_users.insert(user_neighbors.begin(), user_neighbors.end());
        iter++;
		user_neighbors.clear();
    }

	// get the connected_set M, directly
	set<string> connected_set = country_map[id];   //  id's neighbors

	// get the unconnected set N   /*取差集运算*/
	set<string> unconnected_set;                     // N : those nodes who are not id's neighbors    
	set_difference(all_users.begin(), all_users.end(),connected_set.begin(), connected_set.end(),inserter( unconnected_set, unconnected_set.begin() ) );  
	unconnected_set.erase(unconnected_set.find(id)); // delete id itself

	if(unconnected_set.size() == 0){
		recommended_id = "None";
	}else{
		// set the overall variable
		int highest_degree = 0;         
		int most_common_nbr_n = 0;
		set<string>::iterator it;  
		string first_one = *(unconnected_set.begin());
		string possible_recommend_id_judged_by_degree = first_one;
		string possible_recommend_id_judged_by_common_nbrs = first_one;

		// traverse the N (unconnected_set) to find the biggest "common_nbr_n" or biggest "degree"
		for (it = unconnected_set.begin(); it != unconnected_set.end(); ++it) {
    		string n = *it;                             //get every n from N set (unconnected_set)
			set<string> n_neighbors = country_map[n];   //get n's neighbors

			// recrod the n has the largest degree. :  when every common_nbr_n of n  is 0 , we will use it
			if (highest_degree < n_neighbors.size()){
				highest_degree = n_neighbors.size();
				possible_recommend_id_judged_by_degree = n;
			}else if(highest_degree == n_neighbors.size()){   //when meet the same degree， 
				if (possible_recommend_id_judged_by_degree > n){
					possible_recommend_id_judged_by_degree.clear();
					possible_recommend_id_judged_by_degree = n;
				}
			}

			// calculate the possible most_common_neghbors of every n 
			set<string>::iterator sub_it;        //traverse the n_neighbors
			int candidate_most_common_nbr_n = 0;
			for(sub_it = n_neighbors.begin(); sub_it != n_neighbors.end(); ++sub_it){
				string n_nbr = *sub_it;          //one n's neighbor
				if(connected_set.find(n_nbr) != connected_set.end()){  // if this n's neighbor also in id's neighbors
					candidate_most_common_nbr_n ++;
				}
			}

			// record the n in N has the most common nbrs with u (id) 
			if(candidate_most_common_nbr_n > most_common_nbr_n){
				int temp = most_common_nbr_n;
				most_common_nbr_n = candidate_most_common_nbr_n;
				possible_recommend_id_judged_by_common_nbrs = n;
			}else if(candidate_most_common_nbr_n == most_common_nbr_n){ // when the candidate has same common_nbr_n with the previous most_common_nbr_n
				if (possible_recommend_id_judged_by_common_nbrs > n){   //  choose the smaller id
					possible_recommend_id_judged_by_common_nbrs.clear();
					possible_recommend_id_judged_by_common_nbrs = n;
				}
			}
			// in the end : 
			// if most_common_nbr_n is 0 , we recommend by degree, else, we recommend by most_common_neghbors
			if(most_common_nbr_n == 0){
				recommended_id = possible_recommend_id_judged_by_degree;
			}else{
				recommended_id = possible_recommend_id_judged_by_common_nbrs;
			}
		}
	}
}



int main (){
	getdata();      // get data from test1.txt
	UDP_init();     // initial the UDP
	receive_data(); //  receive the command from the main_server
	send_data(all_countries_names);  // send data of all countries to main server
	cout << "The server A has sent a country list to Main Server" << endl;

	while(true){
		string country_name, id;
		receive_data();
		istringstream ss(buf);

		ss >> country_name;
		ss >> id;

		// cout<< "---------------------" << endl;
		// cout << "country_name from Main server to A is " << country_name << endl;
		// cout << "id from Main server to A is " << id << endl;
		// cout<< "---------------------" << endl;

		if(!country_name.empty() && !id.empty()){  // if string not empty
			cout << "The server A has received request for finding possible friends of User "<< id << " in "<< country_name << endl;
		}

		map<string, map<string, set<string> > >::iterator it = all_data.find(country_name);   // find the country_name if in the map key 
 
		if(it != all_data.end()){ //if we find the country name (that is to say, the iterator does not get to end)
			map<string, set<string> > country_map = it->second;    
			map<string, set<string> >::iterator sub_it = country_map.find(id);  // sub iterator , find the id in country_map's key
			
			if(sub_it != country_map.end()) {              // if we find this id 
				cout<< "The server A is searching possible friends for User " << id << "...." << endl;

				// set<string> id_nbr_set = sub_it->second;   // this is this id' s neighbors set
				// set<string>::iterator sub_sub_iter;
				// string neighbor_users = "";   //////--------??????????????  we will print all this id's neighbors
				// for(sub_sub_iter = id_nbr_set.begin(); sub_sub_iter != id_nbr_set.end(); sub_sub_iter++) {
				// 	neighbor_users += *sub_sub_iter;  
				// 	neighbor_users += " ";
				// } 
				//////////////////======
				recommendation_algorithm(country_map, id);
				cout << "Here are the results:" << recommended_id << endl;     ///// ????  需计算出结果
				//////////////////======

				send_data("FIND ServerA " + recommended_id);  ////   ------------- ??????? 
				cout << "The server A has sent the result(s) to Main Server" << endl;
			} else {
				cout<< id << " does not show up in " << country_name << endl;
				send_data("NO_ID");
				cout<< "The server A has sent \"User "<< id << " not found\" to Main Server" << endl;
			}
		}else{
			send_data("NO_COUNTRY");
		}
	}
}
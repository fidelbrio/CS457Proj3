#include <iostream>
#include <fstream>
#include <mutex>
#include <stdio.h>
#include <sys/types.h>
#include <pthread.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sstream>
#include <algorithm>
#include <thread>
#include <chrono>
#include <random>
#include "bank.pb.h"

using namespace std;

std::mutex myMutex;
BranchMessage message;
InitBranch currBranch;
Transfer trans;
string currName;
string currIp;
int currPort;

map <string, int> branchList;


void transferRec(char *source, int amount){

	myMutex.lock();
	myMutex.unlock();

}

void transferSend(){
	

}



int main(int argc, char* argv[]){
	GOOGLE_PROTOBUF_VERIFY_VERSION;

	if(argc != 3){
		cerr << "Usage: " << "argv[0] << "./branch <branch_name> <port_number>" << endl;
		return -1;
	}

	currName = argv[1];
	currPort = atoi(argv[2]);

	struct sockaddr_in socket_address;
	socklen_t address_len = sizeof(socket_address);

	int server_ds;
	int my_socket;
	int read_count;

	char socket_buffer[4096] = {0};

	if ((server_ds = socket(AF_INET, SOCK_STREAM, 0)) <= 0){
        cerr << "ERROR: failed in creating socket" << endl;
        exit(EXIT_FAILURE);
	}
   	if (setsockopt(server_ds, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &option, sizeof(option))){
	        cerr << "ERROR: failed setsockopt" << endl;
        	exit(EXIT_FAILURE);
    	}
    	socket_address.sin_family = AF_INET;
    	socket_address.sin_addr.s_addr = INADDR_ANY;
    	socket_address.sin_port = htons(currPort);

    	if (bind(server_ds, (struct sockaddr *)&socket_address, sizeof(socket_address))<0){
        	cerr << "ERROR: binding failed" << endl;
        	exit(EXIT_FAILURE);
    	}
	if (listen(server_ds, 20) < 0){
        	cerr << "ERROR: failed at listen" << endl;
        	exit(EXIT_FAILURE);
	}


	while(1){
		if((my_socket = accept(server_ds, (struct sockaddr *) &socket_address, (socklen_t*) &address_len)) <0){
			cerr << "ERROR: failed to accept" << endl;
			exit(EXIT_FAILURE);
		}

		read_count = read(my_socket, socket_buffer, 4096);
		if(read_count < 0){
			cerr << "ERROR: failed to read socket_buffer" << endl;
			return -1;
		}

		socket_buffer[read_count] = '\0';
		message.ParseFromString(socket_buffer);

		if(message.has_init_branch()){
			currBranch = message.init_branch();
			
			for(int i = 0; i < currBranch.all_branches_size(); i++){
				const InitBranch_Branch addThis = currBranch.all_branches(i);
				branchList.insert(pair<string, int>(addThis.name(), 0));
			}
		}

		if(message.has_transfer()){
			trans = message.transfer();

			myMutex.lock();
			currBranch.set_balance(currBranch.balance() + trans.amount());
			myMutex.unlock();
		}



	}

}

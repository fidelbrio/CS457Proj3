#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
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
#include <time.h>
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
int initBal;

map <string, int> branchList;


void transferRec(char *source, int amount){

	myMutex.lock();
	myMutex.unlock();

}

void transferSend(){
	// time delay first
	srand(time(0));
	int delay;
	delay = rand() % 5000 + 1;
	cout << "random delay: " << delay << endl;
	std::this_thread::sleep_for(std::chrono::milliseconds(delay));

	int sendAmount;
	int sendPercent;
	sendPercent = rand() % 5 + 1;
	cout << "random percent selected: " << sendPercent << endl;

	sendAmount = (sendPercent * initBal) / 100;
	if(sendAmount < currBranch.balance()){
		myMutex.lock();
		currBranch.set_balance(currBranch.balance() - sendAmount);
		myMutex.unlock();
	} else{
		cout << "This branch does not have enough money to transfer that much" << endl;
	}

	// Now transfer this money to another branch
	

}



int main(int argc, char* argv[]){
	GOOGLE_PROTOBUF_VERIFY_VERSION;

	if(argc != 3){
		cerr << "Usage: " << argv[0] << "./branch <branch_name> <port_number>" << endl;
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
	int option = 1;
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


	struct hostent *hostIP;
	char buf[256];
	gethostname(buf,sizeof(buf));
	hostIP = gethostbyname(buf);
	currIp = inet_ntoa(*((struct in_addr*)hostIP->h_addr_list[0]));
	cout<<currIp<<endl;
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
		cout<<"We just read"<<endl;
		socket_buffer[read_count] = '\0';
		message.ParseFromString(socket_buffer);

		if(message.has_init_branch()){
			currBranch = message.init_branch();
			for(int i = 0; i < currBranch.all_branches_size(); i++){
				const InitBranch_Branch addThis = currBranch.all_branches(i);
				branchList.insert(pair<string, int>(addThis.name(), 0));
				initBal = currBranch.balance();
			}
			cout<<"we got the init branch"<<endl;

		}

		if(message.has_transfer()){
			cout<<"We got the transfer"<<endl;
			trans = message.transfer();

			myMutex.lock();
			currBranch.set_balance(currBranch.balance() + trans.amount());
			myMutex.unlock();
		}
		cout<<"closing socket"<<endl;
		close(my_socket);
		close(server_ds);
		exit(0);

	}

}

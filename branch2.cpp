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
#include <thread>

using namespace std;

std::mutex myMutex;
BranchMessage message;
InitBranch currBranch;
Transfer trans;
Marker mark;
string currName;
string currIp;
int currPort;
int initBal;
bool isInitialized = false;
map <string, int> branchList;

map <int, ReturnSnapshot_LocalSnapshot> snapshot; //saved state goes here
map <string, bool> markerSource;
map <string, int> channels;

int markerOut;
int markerIn;

void initSnap(InitSnapshot init){
	BranchMessage initializer;
	ReturnSnapshot_LocalSnapshot currState;
	currState.set_balance(currBranch.balance());
	currState.set_snapshot_id(init.snapshot_id());
	for(auto itr = channels.begin(); itr!= channels.end(); itr++){
		itr->second = 0;
	}
	for(int i = 0; i<currBranch.all_branches_size(); i++){
		InitBranch_Branch workingWith = currBranch.all_branches(i);
		if(currPort != workingWith.port()){
			auto itr = markerSource.find(workingWith.name());
			if(itr == markerSource.end()){
				cout<<"name of branch not found in marker source" <<endl;
			}else{
				itr->second = true;
			}
		}
	}
	pair<int, ReturnSnapshot_LocalSnapshot> newSnap(init.snapshot_id(),currState);
	snapshot.insert(newSnap);
	markerOut = 0;
	markerIn = 0;
	for(int i = 0; i<currBranch.all_branches_size(); i++){
		InitBranch_Branch targetBranch = currBranch.all_branches(i);
		if(currPort != targetBranch.port()){
			markerOut++;
			int n = 1;
                	struct sockaddr_in addr;
                	int socc = socket(AF_INET, SOCK_STREAM, 0);
                	if(socc < 0){
                		cout<<"Error establishing socket"<<endl;
                        	exit(0);
                	}
                	setsockopt(socc,SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &n, sizeof(n));
                //        cout<<"Check a"<<endl;
                	memset((char *)&addr, 0, sizeof(addr));
                	addr.sin_family = AF_INET;
                        addr.sin_addr.s_addr = htonl(INADDR_ANY);
                        addr.sin_port =htons(targetBranch.port());
                        int binder = bind(socc, (struct sockaddr *)&addr, sizeof(addr));
                        if(binder < 0){
                                cout<<"Error with binder"<<endl;
                                exit(0);
                        }

                        //struct sockaddr_in server;
                        int check = inet_pton(AF_INET, targetBranch.ip().c_str(), &addr.sin_addr);
                        if(check <= 0){
                                cout<<"Error with inet_pton"<<endl;
                                exit(0);
                        }
                        //cout<<"Check b"<<endl;
                        int cnt = connect(socc, (struct sockaddr *)&addr, sizeof (addr));
                        if(cnt < 0){
                                cout<<"Error in connect"<<endl;
                                exit(0);
                        }
			string output;
	        	Marker markerInitializer;
        		markerInitializer.set_send_branch(currName);
        		markerInitializer.set_snapshot_id(init.snapshot_id());
        		initializer.set_allocated_marker(&markerInitializer);
        		initializer.SerializeToString(&output);
			send(socc, output.c_str(), output.size(), 0);
                        close(socc);
		}
	}
	initializer.release_marker();
}

void marker(int snapID, string source){
	cout << "Sending out marker \n" << endl;
	auto temp = snapshot.find(snapID);
	if(temp != snapshot.end()){
		cout << "This marker's been recieved before\n" << endl;

		auto temp2 = markerSource.find(source);
		if(temp2 != markerSource.end()){
			temp2->second = false; // false?
		}

		for(auto itr = channels.begin(); itr != channels.end(); ++itr){
			if(itr->first == source){
				if(itr->second == 0){
					temp->second.add_channel_state(1);
				}else{
					temp->second.add_channel_state(1+itr->second);
				}
			}
		}
	}else{
		cout << "This is the first time we see this marker\n" << endl;

		BranchMessage initSnap;
		ReturnSnapshot_LocalSnapshot localState;
		localState.set_snapshot_id(snapID);
		localState.set_balance(currBranch.balance());
		localState.clear_channel_state();
		for(auto itr = channels.begin(); itr != channels.end(); ++itr){
			itr->second = 0;
		}
		snapshot.insert(pair<int, ReturnSnapshot_LocalSnapshot> (localState.snapshot_id(), localState));

		markerOut = 0;
		markerIn = 0;
		string output;
		Marker initMarker;
		initMarker.set_snapshot_id(localState.snapshot_id());
		initSnap.set_allocated_marker(&initMarker);
		for(int i=0; i < currBranch.all_branches_size(); i++){
			//sending marker to each branch

			InitBranch_Branch branchG = currBranch.all_branches(i); //we are sending out this branch
			bool sent;

			if(currPort != branchG.port()){
				cout << currPort << " sending Marker to " << branchG.port() << endl;
				markerOut++;
				auto temp2 = markerSource.find(branchG.name());
				if(temp2 != markerSource.end()){
					temp2->second = true;
				} //else?
			}
			int n = 1;
                	struct sockaddr_in addr;
                	int socc = socket(AF_INET, SOCK_STREAM, 0);
                	if(socc < 0){
                		cout<<"Error establishing socket"<<endl;
                        	exit(0);
                	}
                	setsockopt(socc,SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &n, sizeof(n));
                //        cout<<"Check a"<<endl;
                	memset((char *)&addr, 0, sizeof(addr));
                	addr.sin_family = AF_INET;
                        addr.sin_addr.s_addr = htonl(INADDR_ANY);
                        addr.sin_port =htons(branchG.port());
                        int binder = bind(socc, (struct sockaddr *)&addr, sizeof(addr));
                        if(binder < 0){
                                cout<<"Error with binder"<<endl;
                                exit(0);
                        }

                        //struct sockaddr_in server;
                        int check = inet_pton(AF_INET, branchG.ip().c_str(), &addr.sin_addr);
                        if(check <= 0){
                                cout<<"Error with inet_pton"<<endl;
                                exit(0);
                        }
                        //cout<<"Check b"<<endl;
                        int cnt = connect(socc, (struct sockaddr *)&addr, sizeof (addr));
                        if(cnt < 0){
                                cout<<"Error in connect"<<endl;
                                exit(0);
                        }			
                        initSnap.SerializeToString(&output);
                        send(socc,output.c_str(),output.size(),0);
                        close(socc);


			//cout << "Pretend were sending messages right now" << endl;


		}
	}

}


void transferSend(int milli){
	cout<<"In transfer function\n"<<endl;
	while(!isInitialized){}
	while(1){
		srand(time(0));
		int delay = (rand()%milli) + 1;
		cout<<"random delay: "<< delay <<endl;
		this_thread::sleep_for(chrono::milliseconds(delay));
		int sendPercent = (rand()%5)+1;
		cout<<"random percent selected "<<sendPercent <<endl;
		int sendAmount = int(currBranch.balance()*(sendPercent/100.0));
		cout<<"send amount selected " << sendAmount <<"\n\n"<<endl;
		if(sendAmount < currBranch.balance()){
                	myMutex.lock();
                	currBranch.set_balance(currBranch.balance() - sendAmount);
                	myMutex.unlock();
        	}else{
                	cout << "This branch does not have enough money to transfer that much" << endl;
			exit(0);
        	}
		int randBranchIndex = rand() % currBranch.all_branches().size();
		InitBranch_Branch targetBranch = currBranch.all_branches(randBranchIndex);
		if(currPort != targetBranch.port()){
			Transfer moneyGram;
			BranchMessage moneyToBranch;
			moneyGram.set_amount(sendAmount);
			moneyGram.set_send_branch(currName);
			moneyToBranch.set_allocated_transfer(&moneyGram);
			int n = 1;
			struct sockaddr_in addr;
			int socc = socket(AF_INET, SOCK_STREAM, 0);
			if(socc < 0){
				cout<<"Error establishing socket"<<endl;
				exit(0);
			}
			setsockopt(socc,SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &n, sizeof(n));
			//cout<<"Check a"<<endl;
			memset((char *)&addr, 0, sizeof(addr));
			addr.sin_family = AF_INET;
        		addr.sin_addr.s_addr = htonl(INADDR_ANY);
        		addr.sin_port =htons(targetBranch.port());
			int binder = bind(socc, (struct sockaddr *)&addr, sizeof(addr));
                	if(binder < 0){
                        	cout<<"Error with binder"<<endl;
                        	exit(0);
                	}

			//struct sockaddr_in server;
        		int check = inet_pton(AF_INET, targetBranch.ip().c_str(), &addr.sin_addr);
			if(check <= 0){
				cout<<"Error with inet_pton"<<endl;
				exit(0);
			}
			//cout<<"Check b"<<endl;
			int cnt = connect(socc, (struct sockaddr *)&addr, sizeof (addr));
			if(cnt < 0){
				cout<<"Error in connect"<<endl;
				exit(0);
			}
			string outputMessage;
			moneyToBranch.SerializeToString(&outputMessage);
			send(socc, outputMessage.c_str(), outputMessage.size(), 0);
			cout<<"just sent"<<endl;
			close(socc);
			cout<<"just closed socc"<<endl;
			moneyToBranch.release_transfer();
			//exit(0);
		}

	}
	return;
}






int main(int argc, char* argv[]){
	GOOGLE_PROTOBUF_VERIFY_VERSION;

	if(argc != 4){
		cerr << "Usage: " << argv[0] << " <branch_name> <port_number> <transfer_interval>" << endl;
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
	int delay = stoi(argv[3]);
	thread transferThread(transferSend,delay);
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
		//cout<<"We just read"<<endl;
		socket_buffer[read_count] = '\0';
		message.ParseFromString(socket_buffer);

		if(message.has_init_branch()){
			cout<<"We are about to be initialized\n"<<endl;
			currBranch = message.init_branch();
			for(int i = 0; i < currBranch.all_branches_size(); i++){
				const InitBranch_Branch addThis = currBranch.all_branches(i);
				//branchList.insert(pair<string, int>(addThis.name(), 0));
				markerSource.insert(pair<string, bool>(addThis.name(), false));
				channels.insert(pair<string, int>(addThis.name(), 0));
				initBal = currBranch.balance();
			}
			cout<<"Branch just became initialized\n"<<endl;
			cout<<currBranch.balance()<<endl;
			isInitialized = true;
		}

		if(message.has_transfer()){
			cout<<"We are receveing a transfer\n"<<endl;
			trans = message.transfer();
			cout<<"We just got $"<<trans.amount() << " from " << trans.send_branch()<<endl;
			myMutex.lock();
			currBranch.set_balance(currBranch.balance() + trans.amount());
			myMutex.unlock();
		}

		if(message.has_marker()){
			cout << "We received a marker!" << endl;
			mark = message.marker();

			int length = message.marker().send_branch().length();
			char name[length+1];
			strcpy(name, message.marker().send_branch().c_str());

			cout << "We just got a marker from " << message.marker().send_branch() << endl;
			//marker(message.marker().snapshot_id(), message.marker().send_branch());
			marker(mark.snapshot_id(), mark.send_branch());

		}

		if(message.has_retrieve_snapshot()){
			cout << "retrieve snapshot" << endl;
			BranchMessage reply;
			ReturnSnapshot returnSnap;

			auto tempID = snapshot.find(message.retrieve_snapshot().snapshot_id());
			if(tempID != snapshot.end()){
				returnSnap.set_allocated_local_snapshot(&tempID->second);
			}
			reply.set_allocated_return_snapshot(&returnSnap);

			//send message here
		}

		if(message.has_init_snapshot()){
			cout<<"snapshot just got initiated"<<endl;
			InitSnapshot init = message.init_snapshot();
			//int id = init.snapshot_id();
			initSnap(init);
		}
		cout<<"closing socket"<<endl;
		close(my_socket);
		//close(server_ds);
		//exit(0);

	}
	transferThread.join();

}

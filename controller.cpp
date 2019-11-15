#include <thread>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <fstream>
#include <string>
#include "bank.pb.h"
#include <vector>
#include <list>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <random>
#include <time.h>

using namespace std;

int numBranches = 0;
InitBranch message;

void retrieveSnapshot(int snapID){
	cout << "Retrieve Snapshot\n\n" << endl;
	RetrieveSnapshot ret;
	BranchMessage sendThis;

	ret.set_snapshot_id(snapID);
	sendThis.set_allocated_retrieve_snapshot(&ret);
	string output;
	sendThis.SerializeToString(&output);
	for(int i=0; i < message.all_branches_size(); i++){
			//sending marker to each branch

			InitBranch_Branch branchG = message.all_branches(i); //we are sending out this branch
			//bool sent;

			//if(currPort != branchG.port()){
				//cout << currPort << " sending Marker to " << branchG.port() << endl;
				//markerOut++;
				//auto temp2 = markerSource.find(branchG.name());
				//if(temp2 != markerSource.end()){
				//	temp2->second = true;
				//} //else?
			//}
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
                        //initSnap.SerializeToString(&output);
                        send(socc,output.c_str(),output.size(),0);
			char inBuff[5000] = {0};
			int weGotIt = read(socc, inBuff, 5000);
			if(weGotIt < 0){
				cout<<"Error in reading in response" <<endl;
				exit(0);
			}else{
				inBuff[weGotIt] = '\0'; //delimiting message by how many bytes we read
				BranchMessage whatWeGot;
				whatWeGot.ParseFromString(inBuff);
				if(whatWeGot.has_return_snapshot()){
					ReturnSnapshot curr = whatWeGot.return_snapshot();
					if(curr.has_local_snapshot()){
						ReturnSnapshot_LocalSnapshot innerCurr = curr.local_snapshot();
						//NOW WE CAN PRINT OUT THE SNAPSHOT OF THE BRANCHES;
					}
				}
                        close(socc);
                 }
	}
	sendThis.release_retrieve_snapshot();
}

void InitBranch(string line, int amount){
	istringstream iss(line);
	vector<string> branchInfo{istream_iterator<string>{iss}, istream_iterator<string>{}};
	InitBranch_Branch * newestBranch = message.add_all_branches();
	newestBranch->set_name(branchInfo[0]);
	newestBranch->set_ip(branchInfo[1]);
	newestBranch->set_port(stoi(branchInfo[2]));
	return;
}

int main(int argc, char * argv[]){
	GOOGLE_PROTOBUF_VERIFY_VERSION;
	if(argc != 3){
		cout<<"Usage: "<<argv[0] << " <initial_money> <input_branch_file>" <<endl;
		exit(0);
	}


	//setting variables from stdin
	int amount = stoi(argv[1]);
	string line;
	ifstream book(argv[2]);
	if(!book.is_open()){
		cout<<"Error opening branches.txt file" <<endl;
		exit(0);
	}else{
		//cout<<"File just got opened"<<endl;
		while(getline(book,line)){
			InitBranch(line, amount);
			cout<<line<<endl;
			numBranches++;
		}
	}
	book.close();
	//cout<<"book checker"<<endl;
	int divAm = amount/numBranches;
	message.set_balance(divAm);
	string toBeSent;
	BranchMessage initializer;
	initializer.set_allocated_init_branch(&message);
	initializer.SerializeToString(&toBeSent);
	initializer.release_init_branch();
	//cout<<"initializer check" <<endl;
	for(int i = 0; i<message.all_branches_size(); i++){
		InitBranch_Branch currBranch = message.all_branches(i);
		int n = 1;
		struct sockaddr_in addr;
		int socc = socket(AF_INET, SOCK_STREAM, 0);
		if(socc < 0){
			cout<<"Error establishing socket"<<endl;
			exit(0);
		}
		setsockopt(socc,SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &n, sizeof(n));
		memset((char *)&addr, 0, sizeof(addr));
		addr.sin_family = AF_INET;
        	addr.sin_addr.s_addr = htonl(INADDR_ANY);
        	addr.sin_port =htons(currBranch.port());
		int binder = bind(socc, (struct sockaddr *)&addr, sizeof(addr));
                if(binder < 0){
                        cout<<"Error with binder"<<endl;
                        exit(0);
                }

		//struct sockaddr_in server;
        	int check = inet_pton(AF_INET, currBranch.ip().c_str(), &addr.sin_addr);
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
		send(socc, toBeSent.c_str(), toBeSent.size(), 0);
		//cout<<"just sent"<<endl;
		close(socc);
		//cout<<"just closed socc"<<endl;
		cout<<"Just Finished Initializing Branch\n\n" <<endl;
	}
	int snapID = 1;

	while(1){

		cout<<"Initiating snapshot\n\n" <<endl;
		srand(time(0));
		int randBranchIndex = rand() % message.all_branches().size();
		InitBranch_Branch targetForSnap = message.all_branches(randBranchIndex);
		int n = 1;
		struct sockaddr_in addr;
		int socc = socket(AF_INET, SOCK_STREAM,0);
		if(socc < 0){
			cout<<"Error establishing socket" <<endl;
			exit(0);
		}
		setsockopt(socc, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &n, sizeof(n));
		memset((char *)&addr, 0, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = htonl(INADDR_ANY);
		addr.sin_port =htons(targetForSnap.port());
		cout<<"Attempted port to start snapshot is " << targetForSnap.port() <<"\n\n"<<endl;
       		int binder = bind(socc, (struct sockaddr *)&addr, sizeof(addr));
        	if(binder < 0){
        		cout<<"Error with binder"<<endl;
                	exit(0);
        	}
        	int check = inet_pton(AF_INET, targetForSnap.ip().c_str(), &addr.sin_addr);
        	if(check <= 0){
        		cout<<"Error with inet_pton"<<endl;
                	exit(0);
        	}
		int cnt = connect(socc, (struct sockaddr *)&addr, sizeof (addr));
       		if(cnt < 0){
        		cout<<"Error in connect: "<< cnt << endl;
			//cout << explain_connect(socc, (struct sockaddr *)&addr, sizeof(addr)) << endl;
			exit(0);
       		}
		string snapStart;
		InitSnapshot initialSnap;
		initialSnap.set_snapshot_id(snapID);
		initializer.set_allocated_init_snapshot(&initialSnap);
		initializer.SerializeToString(&snapStart);
		send(socc,snapStart.c_str(), snapStart.size(), 0);
		cout<<"Message to start snapshot has been sent\n\n"<<endl;
		close(socc);
		initializer.release_init_snapshot();
		this_thread::sleep_for(20s);
		retrieveSnapshot(snapID);
		snapID++;
	}
}

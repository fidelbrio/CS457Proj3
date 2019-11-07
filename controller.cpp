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

using namespace std;

int numBranches = 0;
InitBranch message;


void InitBranch(string line, int amount){
	istringstream iss(line);
	vector<string> branchInfo{istream_iterator<string>{iss}, istream_iterator<string>{}};
	InitBranch_Branch * newestBranch = message.add_all_branches();
	newestBranch->set_name(branchInfo[0]);
	newestBranch->set_ip(branchInfo[1]);
	newestBranch->set_port(stoi(branchInfo[2]));
}

int main(int argc, char * argv[]){
	GOOGLE_PROTOBUF_VERIFY_VERSION;
	if(argc != 3){
		cout<<"Usage: "<<argv[0] << " <initial_money> <input_branch_file>" <<endl;
		exit(0);
	}
	/*int socc = socket(AF_INET, SOCK_STREAM, 0);
	if(socc < 0){
		cout<<"Error establishing socket"<<endl;
		exit(0);
	}
	int n = 1;
	setsockopt(socc,SOL_SOCKET,SO_REUSEADDR | SO_REUSEPORT,&n,sizeof(n));
	struct sockaddr_in addr;
	memset((char *)&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port =htons(8080);*/
	//running on port 8080 SUBJECT TO CHANGE
	


	//setting variables from stdin
	int amount = stoi(argv[1]);
	string line;
	ifstream book(argv[2]);
	if(!book.is_open()){
		cout<<"Error opening branches.txt file" <<endl;
	}else{
		cout<<"File just got opened"<<endl;
		while(getline(book,line)){
			InitBranch(line, amount);
			cout<<line<<endl;
			numBranches++;
		}
	}
	book.close();
	int divAm = amount/numBranches;
	message.set_balance(divAm);
	string toBeSent;
	BranchMessage initializer;
	initializer.set_allocated_init_branch(&message);
	initializer.SerializeToString(&toBeSent);
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
		struct sockaddr_in server;
        	int check = inet_pton(AF_INET, currBranch.ip().c_str(), &server.sin_addr);
		if(check <= 0){
			cout<<"Error with inet_pton"<<endl;
		exit(0);
		int cnt = connect(socc, (struct sockaddr *)&server, sizeof (server));
		if(cnt < 0){
			cout<<"Error in connect"<<endl;
		}
		send(socc, toBeSent.c_str(), toBeSent.size(), 0);
		close(socc);
		}
	}
}


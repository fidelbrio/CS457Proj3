#include <iostream>
#include <stdio.h>
#include <fstream>
#include <string>
#include "bank.pb.h"
#include <vector>
#include <list>

using namespace std;

int numBranches = 0;
list<BranchMessage


void InitBranch(string line, int amount){
	BranchMessage message;
	vector<string> branchInfo{istream_iterator<string>{iss}, istream_iterator<string>{}};
	

int main(int argc, char * argv[]){
	GOOGLE_PROTOBUF_VERIFY_VERSION;
	if(argc != 3){
		cout<<"Usage: "<<argv[0] << " <initial_money> <input_branch_file>" <<endl;
		exit(0);
	}









	int socc = socket(AF_INET, SOCK_STREAM, 0);
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
	addr.sin_port =htons(8080);
	//running on port 8080 SUBJECT TO CHANGE
	


	//setting variables from stdin
	int amount = stoi(argv[1]);
	string line;
	ifstream book(argv[2]);
	if(!book.is_open()){
		cout<<"Error opening branches.txt file" <<endl;
	}else{
		while(getline(book,line)){
			InitBranch(line, amount);
			cout<<line<<endl;
		}
	}
}

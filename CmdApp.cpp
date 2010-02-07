#include "CmdApp.h"

#include <iostream>
#include <iomanip>

bool CmdApp::Run(int argc,char** argv){
	if(argc<2){
		return this->Help();
	}

	string cmd(argv[1]);

	Args args;
	for(int i=2;i<argc;i++){
		args.push_back(argv[i]);
	}

	if(cmd=="--help"){
		return this->Help();
	}

	if(this->cmap.find(argv[1])!=this->cmap.end()){
		return this->cmap[argv[1]]->operator()(args);
	}

	cerr << "Command "<<argv[1]<<" not found"<<endl;

	return false;
}

bool CmdApp::Help(){
	cout << "Usage: "<<this->appname<<" args"<<endl;
	for(CmdMap::iterator cIt=this->cmap.begin();cIt!=this->cmap.end();cIt++){
		cout <<setw(15)<<(*cIt).first<<" - "<<(*cIt).second->getDescription()<<endl;
	}
	return true;
}

CmdApp::~CmdApp(){
	for(CmdMap::iterator cIt=this->cmap.begin();cIt!=this->cmap.end();cIt++){
		delete((*cIt).second);
	}
}

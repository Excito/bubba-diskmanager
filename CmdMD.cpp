/*
 * CmdMD.cpp
 *
 *  Created on: Aug 14, 2009
 *      Author: tor
 */

#include "CmdMD.h"

#include "RaidDevs.h"

#include <sstream>
#include <iomanip>

#include <stdlib.h>

#include <libeutils/json/json.h>

CmdMD::CmdMD():Cmd(""){
	stringstream ss;
	ss << "Manage raid arrays"<<endl;
	ss <<setw(15)<<""<<"   "<<"list [debug]"<<endl;
	ss <<setw(15)<<""<<"   "<<"get_next_md"<<endl;
	ss <<setw(15)<<""<<"   "<<"create 'type' 'disks' 'spares' 'devs...' [devpath]"<<endl;
	ss <<setw(15)<<""<<"   "<<"destroy 'mddev'"<<endl;
	ss <<setw(15)<<""<<"   "<<"assemble 'devs...'"<<endl;
	ss <<setw(15)<<""<<"   "<<"stop 'mddev'"<<endl;
	ss <<setw(15)<<""<<"   "<<"fail 'mddev' 'disk'"<<endl;
	ss <<setw(15)<<""<<"   "<<"remove 'mddev' 'disk'"<<endl;
	ss <<setw(15)<<""<<"   "<<"add 'mddev' 'disk'";
	this->description=ss.str();
}

bool CmdMD::do_list(Args& arg){
	Json::Value ret=Json::Value(Json::arrayValue);
	RaidDevs rd;

	list<Device> devs=rd.GetMDs();

	for(list<Device>::iterator dIt=devs.begin();dIt!=devs.end();dIt++){
		Json::Value disk=fromHash((*dIt).attr);
		if ((*dIt).slaves.size()>0){
			disk["disks"]=Json::Value(Json::arrayValue);
			for(list<Hash>::iterator pIt=(*dIt).slaves.begin();pIt!=(*dIt).slaves.end();pIt++){
				disk["disks"].append(
						fromHash(*pIt));
			}
		}
		ret.append(disk);
	}

	if(arg.size()>0){
		if(arg[0]=="debug"){
			cout << ret.toStyledString();
		}else{
			return false;
		}
	}else{
		cout << Json::FastWriter().write(ret);
	}
	return true;
}

bool CmdMD::do_create(Args& arg){
	Json::Value ret=Json::Value(Json::objectValue);
	RaidDevs rd;
	if(arg.size()<5){
		ret["status"]=false;
		ret["errmsg"]="To few arguments";
		cout << Json::FastWriter().write(ret);
		return false;
	}

	RaidDevs::types level=atoi(arg[0].c_str())==0?RaidDevs::RAID0:RaidDevs::RAID1;

	int ndisks=atoi(arg[1].c_str());
	int nspare=atoi(arg[2].c_str());
	arg.erase(arg.begin(),arg.begin()+3);

	if(arg.size()>static_cast<size_t>(ndisks+nspare+1)){
		ret["status"]=false;
		ret["errmsg"]="To many arguments";
		cout << Json::FastWriter().write(ret);
		return false;
	}

	if(arg.size()<static_cast<size_t>(ndisks+nspare)){
		ret["status"]=false;
		ret["errmsg"]="To few device arguments";
		cout << Json::FastWriter().write(ret);
		return false;
	}

	list<string> devs;
	for(int i=0;i<ndisks+nspare;i++){
		devs.push_back(arg[i]);
	}
	arg.erase(arg.begin(),arg.begin()+ndisks+nspare);

	bool retval;
	if(arg.size()){
		retval=rd.CreateMD(level,ndisks,nspare,devs,arg[0]);
	}else{
		retval=rd.CreateMD(level,ndisks,nspare,devs);
	}
	ret["status"]=retval;
	if(!retval){
		ret["errmsg"]="Operation failed";
	}
	cout << Json::FastWriter().write(ret);

	return retval;
}

bool CmdMD::do_get_next_md(){
	Json::Value ret=Json::Value(Json::objectValue);

	RaidDevs rd;

	string next = rd.GetNextMD();

	ret["nextmd"] = next;

	cout << Json::FastWriter().write(ret);

	return true;
}

bool CmdMD::do_assemble(Args& arg){
	Json::Value ret=Json::Value(Json::objectValue);
	bool rval=true;

	if(arg.size()<1){
		ret["errmsg"]="Wrong amt of args";
		rval=false;
	}

	if(rval){
		list<string> devs;
		for(int i=0;i<arg.size();i++){
			devs.push_back(arg[i]);
		}
		RaidDevs rd;
		if(!rd.AssembleMD(devs)){
			rval=false;
			ret["errmsg"]="Operation failed";
		}
	}

	ret["status"]=rval;
	cout << Json::FastWriter().write(ret);
	return rval;
}

bool CmdMD::do_stop(Args& arg){
	Json::Value ret=Json::Value(Json::objectValue);
	bool rval=true;

	if(arg.size()!=1){
		ret["errmsg"]="Wrong amt of args";
		rval=false;
	}

	if(rval){
		RaidDevs rd;
		if(!rd.StopMD(arg[0])){
			rval=false;
			ret["errmsg"]="Operation failed";
		}
	}

	ret["status"]=rval;
	cout << Json::FastWriter().write(ret);
	return rval;
}

bool CmdMD::do_destroy(Args& arg){
	Json::Value ret=Json::Value(Json::objectValue);
	bool rval=true;

	if(arg.size()!=1){
		ret["errmsg"]="Wrong amt of args";
		rval=false;
	}

	if(rval){
		RaidDevs rd;
		if(!rd.DestroyMD(arg[0])){
			rval=false;
			ret["errmsg"]="Operation failed";
		}
	}

	ret["status"]=rval;
	cout << Json::FastWriter().write(ret);
	return rval;
}

bool CmdMD::do_fail(Args& arg){
	Json::Value ret=Json::Value(Json::objectValue);
	bool rval=true;

	if(arg.size()!=2){
		ret["errmsg"]="Wrong amt of args";
		rval=false;
	}

	if(rval){
		RaidDevs rd;
		if(!rd.FailDisk(arg[0],arg[1])){
			rval=false;
			ret["errmsg"]="Operation failed";
		}
	}

	ret["status"]=rval;
	cout << Json::FastWriter().write(ret);
	return rval;
}

bool CmdMD::do_remove(Args& arg){
	Json::Value ret=Json::Value(Json::objectValue);
	bool rval=true;

	if(arg.size()!=2){
		ret["errmsg"]="Wrong amt of args";
		rval=false;
	}

	if(rval){
		RaidDevs rd;
		if(!rd.RemoveDisk(arg[0],arg[1])){
			rval=false;
			ret["errmsg"]="Operation failed";
		}
	}

	ret["status"]=rval;
	cout << Json::FastWriter().write(ret);
	return rval;
}

bool CmdMD::do_add(Args& arg){
	Json::Value ret=Json::Value(Json::objectValue);
	bool rval=true;

	if(arg.size()!=2){
		ret["errmsg"]="Wrong amt of args";
		rval=false;
	}

	if(rval){
		RaidDevs rd;
		if(!rd.AddDisk(arg[0],arg[1])){
			rval=false;
			ret["errmsg"]="Operation failed";
		}
	}

	ret["status"]=rval;
	cout << Json::FastWriter().write(ret);
	return rval;
}


bool CmdMD::operator()(Args& arg){
	if(arg.size()==0){
		return false;
	}

	string cmd=arg[0];
	arg.erase(arg.begin());

	if(cmd=="list"){
		return this->do_list(arg);
	}else if(cmd=="get_next_md"){
		return this->do_get_next_md();
	}else if(cmd=="create"){
		return this->do_create(arg);
	}else if(cmd=="destroy"){
		return this->do_destroy(arg);
	}else if(cmd=="assemble"){
		return this->do_assemble(arg);
	}else if(cmd=="stop"){
		return this->do_stop(arg);
	}else if(cmd=="fail"){
		return this->do_fail(arg);
	}else if(cmd=="remove"){
		return this->do_remove(arg);
	}else if(cmd=="add"){
		return this->do_add(arg);
	}else{
		return false;
	}
	return true;
}

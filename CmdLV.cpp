/*
 * CmdLV.cpp
 *
 *  Created on: Aug 14, 2009
 *      Author: tor
 */

#include "CmdLV.h"
#include "LVM.h"

#include <sstream>
#include <iomanip>

#include <libeutils/json/json.h>
#include <libeutils/FileUtils.h>
#include <libeutils/EExcept.h>

CmdLV::CmdLV():Cmd(""){
	stringstream ss;
	ss << "Manage and create logical volumes"<<endl;
	ss <<setw(15)<<""<<"   "<<"list [debug]"<<endl;
	ss <<setw(15)<<""<<"   "<<"pvcreate 'device'"<<endl;
	ss <<setw(15)<<""<<"   "<<"pvremove 'device'"<<endl;
	ss <<setw(15)<<""<<"   "<<"vgcreate 'vgname' 'device' ['device'..]"<<endl;
	ss <<setw(15)<<""<<"   "<<"vgremove 'vgname'"<<endl;
	ss <<setw(15)<<""<<"   "<<"vgextend 'vgname' 'device'"<<endl;
	ss <<setw(15)<<""<<"   "<<"lvcreate 'lvname' 'vgname'"<<endl;
	ss <<setw(15)<<""<<"   "<<"lvremove 'lvpath'"<<endl;
	ss <<setw(15)<<""<<"   "<<"lvextend 'lvpath'";

	this->description=ss.str();
}

bool CmdLV::do_list(Args& arg){
	Json::Value ret=Json::Value(Json::arrayValue);
	LVM lvm;

	list<Device> devs=lvm.GetDevices();

	for(list<Device>::iterator dIt=devs.begin();dIt!=devs.end();dIt++){
		Json::Value lv=fromHash((*dIt).attr);
		if ((*dIt).slaves.size()>0){
			lv["pvs"]=Json::Value(Json::arrayValue);
			for(list<Hash>::iterator pIt=(*dIt).slaves.begin();pIt!=(*dIt).slaves.end();pIt++){
				lv["pvs"].append(
						fromHash(*pIt));
			}
		}
		if ((*dIt).logics.size()>0){
			lv["lvs"]=Json::Value(Json::arrayValue);
			for(list<Hash>::iterator pIt=(*dIt).logics.begin();pIt!=(*dIt).logics.end();pIt++){
				Hash p = (*pIt);
				stringstream devpath;
				devpath << "/dev/" << p["group"] <<  "/" << p["name"];
				Json::Value lvss = fromHash(*pIt);
				lvss["devices"] = Json::Value(Json::arrayValue);
				stringlist sl = dIt->logics_devices[devpath.str()];
				for(stringlist::iterator sIt=sl.begin();sIt!=sl.end();++sIt){
					lvss["devices"].append(*sIt);
				}
				lv["lvs"].append(lvss);
			}
		}
		ret.append(lv);
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

bool CmdLV::do_pvcreate(Args& arg){
	Json::Value ret=Json::Value(Json::objectValue);
	bool rval=true;

	if(arg.size()!=1){
		ret["errmsg"]="Wrong amt of arguments";
		rval=false;
	}

	if(rval){
		try{
			if(!S_ISBLK(EUtils::Stat(arg[0]).GetMode())){
				ret["errmsg"]="Not a blockdevice";
				rval=false;
			}
		}catch(EUtils::EExcept::ENoent e){
			ret["errmsg"]="Not a blockdevice";
			rval=false;
		}
	}

	if(rval){
		LVM lvm;
		if(!lvm.CreatePV(arg[0])){
			rval=false;
			ret["errmsg"]="Operation failed";
		}
	}

	ret["status"]=rval;
	cout << Json::FastWriter().write(ret);
	return rval;
}

bool CmdLV::do_pvremove(Args& arg){
	Json::Value ret=Json::Value(Json::objectValue);
	bool rval=true;

	if(arg.size()!=1){
		ret["errmsg"]="Wrong amt of arguments";
		rval=false;
	}

	if(rval){
		try{
			if(!S_ISBLK(EUtils::Stat(arg[0]).GetMode())){
				ret["errmsg"]="Not a blockdevice";
				rval=false;
			}
		}catch(EUtils::EExcept::ENoent e){
			ret["errmsg"]="Not a blockdevice";
			rval=false;
		}
	}

	if(rval){
		LVM lvm;
		if(!lvm.RemovePV(arg[0])){
			rval=false;
			ret["errmsg"]="Operation failed";
		}
	}

	ret["status"]=rval;
	cout << Json::FastWriter().write(ret);
	return rval;
}

bool CmdLV::do_vgcreate(Args& arg){
	Json::Value ret=Json::Value(Json::objectValue);
	bool rval=true;

	if(arg.size()<2){
		ret["errmsg"]="Wrong amt of arguments";
		rval=false;
	}

	if(rval){
		string vgname=arg[0];
		arg.erase(arg.begin(),arg.begin()+1);
		//TODO: validate better.
		list<string> devs;
		for(size_t i=0;i<arg.size();i++){
			devs.push_back(arg[i]);
		}
		LVM lvm;
		if(!lvm.CreateVG(vgname,devs)){
			rval=false;
			ret["errmsg"]="Operation failed";
		}
	}

	ret["status"]=rval;
	cout << Json::FastWriter().write(ret);
	return rval;
}

bool CmdLV::do_vgremove(Args& arg){
	Json::Value ret=Json::Value(Json::objectValue);
	bool rval=true;

	if(arg.size()!=1){
		ret["errmsg"]="Wrong amt of arguments";
		rval=false;
	}
	//TODO: Validate better
	if(rval){
		LVM lvm;
		if(!lvm.RemoveVG(arg[0])){
			rval=false;
			ret["errmsg"]="Operation failed";
		}
	}

	ret["status"]=rval;
	cout << Json::FastWriter().write(ret);
	return rval;
}

bool CmdLV::do_vgextend(Args& arg){
	Json::Value ret=Json::Value(Json::objectValue);
	bool rval=true;

	if(arg.size()!=2){
		ret["errmsg"]="Wrong amt of arguments";
		rval=false;
	}

	if(rval){
		if(rval){
			try{
				if(!S_ISBLK(EUtils::Stat(arg[1]).GetMode())){
					ret["errmsg"]="Not a blockdevice";
					rval=false;
				}
			}catch(EUtils::EExcept::ENoent e){
				ret["errmsg"]="Not a blockdevice";
				rval=false;
			}
		}

	}

	if(rval){
		LVM lvm;
		if(!lvm.ExtendVG(arg[0],arg[1])){
			rval=false;
			ret["errmsg"]="Operation failed";
		}
	}

	ret["status"]=rval;
	cout << Json::FastWriter().write(ret);
	return rval;
}

bool CmdLV::do_lvcreate(Args& arg){
	Json::Value ret=Json::Value(Json::objectValue);
	bool rval=true;

	if(arg.size()!=2){
		ret["errmsg"]="Wrong amt of arguments";
		rval=false;
	}

	if(rval){
		LVM lvm;
		if(!lvm.CreateLV(arg[0],arg[1])){
			rval=false;
			ret["errmsg"]="Operation failed";
		}
	}

	ret["status"]=rval;
	cout << Json::FastWriter().write(ret);
	return rval;
}

bool CmdLV::do_lvremove(Args& arg){
	Json::Value ret=Json::Value(Json::objectValue);
	bool rval=true;

	if(arg.size()!=1){
		ret["errmsg"]="Wrong amt of arguments";
		rval=false;
	}
	//TODO: Validate better
	if(rval){
		LVM lvm;
		if(!lvm.RemoveLV(arg[0])){
			rval=false;
			ret["errmsg"]="Operation failed";
		}
	}

	ret["status"]=rval;
	cout << Json::FastWriter().write(ret);
	return rval;
}

bool CmdLV::do_lvextend(Args& arg){
	Json::Value ret=Json::Value(Json::objectValue);
	bool rval=true;

	if(arg.size()!=1){
		ret["errmsg"]="Wrong amt of arguments";
		rval=false;
	}
	//TODO: Validate better
	if(rval){
		LVM lvm;
		if(!lvm.ExtendLV(arg[0])){
			rval=false;
			ret["errmsg"]="Operation failed";
		}
	}

	ret["status"]=rval;
	cout << Json::FastWriter().write(ret);
	return rval;
}

bool CmdLV::operator()(Args& arg){
	if(arg.size()==0){
		return false;
	}

	string cmd=arg[0];
	arg.erase(arg.begin());

	if(cmd=="list"){
		return this->do_list(arg);
	}else if(cmd=="pvcreate"){
		return this->do_pvcreate(arg);
	}else if(cmd=="pvremove"){
		return this->do_pvremove(arg);
	}else if(cmd=="vgcreate"){
		return this->do_vgcreate(arg);
	}else if(cmd=="vgremove"){
		return this->do_vgremove(arg);
	}else if(cmd=="vgextend"){
		return this->do_vgextend(arg);
	}else if(cmd=="lvcreate"){
		return this->do_lvcreate(arg);
	}else if(cmd=="lvremove"){
		return this->do_lvremove(arg);
	}else if(cmd=="lvextend"){
		return this->do_lvextend(arg);
	}else{
		return false;
	}
	return true;
}

/*
 * DiskCmd.cpp
 *
 *  Created on: Aug 14, 2009
 *      Author: tor
 */

#include "DiskCmd.h"
#include "Disks.h"
#include "RaidDevs.h"
#include "LVM.h"

#include <sstream>
#include <iomanip>

#include <libeutils/json/json.h>
#include <libeutils/FileUtils.h>
#include <libeutils/EExcept.h>


DiskCmd::DiskCmd():Cmd(""){
	stringstream ss;
	ss << "Manage physical disks"<<endl;
	ss <<setw(15)<<""<<"   "<<"list [debug]"<<endl;
	ss <<setw(15)<<""<<"   "<<"partition 'device' 'parttype' [label='Bubba Disk']"<<endl;
	ss <<setw(15)<<""<<"   "<<"set_partition_type 'partition' 'parttype'"<<endl;
	ss <<setw(15)<<""<<"   "<<"format 'device' 'fstype'"<<endl;
	ss <<setw(15)<<""<<"   "<<"extend 'device'";
	ss <<setw(15)<<""<<"   "<<"probe 'device'";
	this->description=ss.str();
}

bool DiskCmd::do_list(Args& arg){
	Disks disks;
	LVM lvm;
	lvm.Mark(disks);
	RaidDevs rd;
	rd.Mark(disks);
	list<Device> devs=disks.GetPhysicalDisks();
	Json::Value ret(Json::arrayValue);
	for(list<Device>::iterator dIt=devs.begin();dIt!=devs.end();dIt++){
		Json::Value disk=fromHash((*dIt).attr);
		if ((*dIt).slaves.size()>0){
			disk["partitions"]=Json::Value(Json::arrayValue);
			for(list<Hash>::iterator pIt=(*dIt).slaves.begin();pIt!=(*dIt).slaves.end();pIt++){
				disk["partitions"].append(
						fromHash(*pIt));
			}
		}
		ret.append(disk);
	}
	if(arg.size()>0){
		if(arg[0]=="debug"){
			cout << ret.toStyledString()<<endl;
		}else{
			return false;
		}
	}else{
		cout<<this->writer.write(ret);
	}
	return true;
}

bool DiskCmd::do_extend(Args& arg){
	Json::Value ret=Json::Value(Json::objectValue);
	bool rval=true;

	if(arg.size()!=1){
		ret["errmsg"]="Wrong amt of args";
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
		Disks disks;

		// Todo: could perhaps have connected directly to our update
		disks.StatusChanged.connect(sigc::mem_fun(this,&DiskCmd::update_extend_status));

		if(!disks.ExtendPartition(arg[0])){
			ret["errmsg"]="Operation failed";
			rval=false;
		}
	}

	ret["status"]=rval;
	cout << Json::FastWriter().write(ret);
	return rval;
}

void DiskCmd::update_format_status(const string& status){
	this->UpdateStatus(status);
}

void DiskCmd::update_extend_status(const string& status){
	this->UpdateStatus(status);
}


bool DiskCmd::do_format(Args& arg){
	Json::Value ret=Json::Value(Json::objectValue);
	bool rval=true;

	if(arg.size()!=2){
		ret["errmsg"]="Wrong amt of args";
		rval=false;
	}

	try{
		if(!S_ISBLK(EUtils::Stat(arg[0]).GetMode())){
			ret["errmsg"]="Not a blockdevice";
			rval=false;
		}
	}catch(EUtils::EExcept::ENoent e){
		ret["errmsg"]="Not a blockdevice";
		rval=false;
	}

	if(rval){
		if(arg[1]=="ext3"){

		}else if(arg[1]=="vfat"){

		}else if(arg[1]=="msdos"){

		}else{
			ret["errmsg"]="Unknown filesystem type";
			rval=false;
		}
	}

	if(rval){
		Disks disks;
		// Todo: could perhaps have connected directly to our update
		disks.StatusChanged.connect(sigc::mem_fun(this,&DiskCmd::update_format_status));
		if(!(rval=disks.FormatPartition(arg[0],arg[1]))){
			ret["errmsg"]="Operation failed";
		}
	}

	ret["status"]=rval;
	cout << Json::FastWriter().write(ret);
	return rval;
}
bool DiskCmd::do_partition(Args& arg){
	Json::Value ret=Json::Value(Json::objectValue);
	bool rval=true;

	if(arg.size() < 2 || arg.size() > 3 ){
		ret["errmsg"]="Wrong amt of args";
		rval=false;
	}

	try{
		if(!S_ISBLK(EUtils::Stat(arg[0]).GetMode())){
			ret["errmsg"]="Not a blockdevice";
			rval=false;
		}
	}catch(EUtils::EExcept::ENoent e){
		ret["errmsg"]="Not a blockdevice";
		rval=false;
	}

	Disks::PartType pt;
	if(rval){
		if(arg[1]=="lvm"){
			pt=Disks::LVM;
		}else if(arg[1]=="raid"){
			pt=Disks::Raid;
		}else if(arg[1]=="raw"){
			pt=Disks::Raw;
		}else{
			rval=false;
			ret["errmsg"]="Unknown partition type";
		}
	}

	string label("Bubba Disk");
	if( arg.size() == 3 ) {
		label = arg[2];
	}

	if(rval){
		try {
			Disks::CreateSimplePart(arg[0],pt, label);
		} catch( Disks::disk_error &e ) {
			rval=false;
			ret["errmsg"] = e.what();
		}
	}

	ret["status"]=rval;
	cout << Json::FastWriter().write(ret);
	return rval;
}

bool DiskCmd::do_set_partition_type(Args& arg){
	Json::Value ret=Json::Value(Json::objectValue);
	bool rval=true;

	if(arg.size()!=2){
		ret["errmsg"]="Wrong amt of args";
		rval=false;
	}
	string device, partition;
	{
		string t_part = arg[0];
		int length = t_part.length();
		if( length < 7 ) {
			ret["errmsg"]="Too short partition definition, should be in the form /dev/sdXN";
			rval=false;
		}
		device = t_part.substr( 0 , length - 1 );
		partition = t_part.substr( length - 1 , 1 );

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

	Disks::PartType pt;
	if(rval){
		if(arg[1]=="lvm"){
			pt=Disks::LVM;
		}else if(arg[1]=="raid"){
			pt=Disks::Raid;
		}else{
			rval=false;
			ret["errmsg"]="Unknown partition type";
		}
	}

	if(rval){
		try {
			Disks::SetPartitionType(device, partition, pt);
		} catch( Disks::disk_error &e ) {
			rval=false;
			ret["errmsg"] = e.what();
		}
	}

	ret["status"]=rval;
	cout << Json::FastWriter().write(ret);
	return rval;
}

bool DiskCmd::do_probe(Args& arg){
	Json::Value ret=Json::Value(Json::objectValue);
	bool rval=true;

	if(arg.size()!=1){
		ret["errmsg"]="Wrong amt of args";
		rval=false;
	}

	if(rval){
		try {
			Disks::Probe(arg[0]);
		} catch( Disks::disk_error &e ) {
			rval=false;
			ret["errmsg"] = e.what();
		}
	}

	ret["status"]=rval;
	cout << Json::FastWriter().write(ret);
	return rval;
}

bool DiskCmd::operator()(Args& arg){
	if(arg.size()==0){
		return false;
	}

	string cmd=arg[0];
	arg.erase(arg.begin());

	if(cmd=="list"){
		return this->do_list(arg);
	}else if(cmd=="partition"){
		return this->do_partition(arg);
	}else if(cmd=="set_partition_type"){
		return this->do_set_partition_type(arg);
	}else if(cmd=="format"){
		return this->do_format(arg);
	}else if(cmd=="extend"){
		return this->do_extend(arg);
	}else if(cmd=="probe"){
		return this->do_probe(arg);
	}else{
		return false;
	}
	return true;
}

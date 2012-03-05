
#include "FsTabCmd.h"

#include "Disks.h"


#include <sstream>
#include <iomanip>

#include <stdlib.h>

#include <libeutils/json/json.h>
#include <libeutils/FsTab.h>
#include <libeutils/StringTools.h>
#include <libeutils/FileUtils.h>
#include <libeutils/EExcept.h>


#define FSTAB "/etc/fstab"

FsTabCmd::FsTabCmd():Cmd(""){
	stringstream ss;

	ss << "Manage fstab"<<endl;
	ss <<setw(15)<<""<<"   "<<"list [debug]"<<endl;
	ss <<setw(15)<<""<<"   "<<"add 'device' 'path' 'fstype' 'options' freq passno"<<endl;
	ss <<setw(15)<<""<<"   "<<"add_by_uuid 'device' 'path' ['options'=defaults]"<<endl;
	ss <<setw(15)<<""<<"   "<<"remove 'device' or 'path'"<<endl;
	ss <<setw(15)<<""<<"   "<<"is_mounted 'device' or 'path'"<<endl;
	ss <<setw(15)<<""<<"   "<<"mount 'device' or 'path'"<<endl;
	ss <<setw(15)<<""<<"   "<<"umount 'device' or 'path'";


	this->description=ss.str();
}

bool FsTabCmd::do_list(Args& arg){
	Json::Value ret=Json::Value(Json::objectValue);
	bool rval=true;

	if(arg.size()>1){
		rval=false;
		ret["errmsg"]="Wrong amt of args";
	}

	if((arg.size()==1) && (arg[0]!="debug")){
		rval=false;
		ret["errmsg"]="Unknown argument to list partitions";
	}

	if(rval){
		EUtils::FsTab tab(FSTAB);
		map<string,EUtils::FsTab::Entry> entries=tab.GetEntries();
		ret["entries"]=Json::Value(Json::arrayValue);
		for(map<string,EUtils::FsTab::Entry>::iterator eIt=entries.begin();eIt!=entries.end();eIt++){
			Json::Value entry=Json::Value(Json::objectValue);

			entry["device"]=(*eIt).second.device;
			entry["mount"]=(*eIt).second.mount;
			entry["fstype"]=(*eIt).second.fstype;
			entry["options"]=(*eIt).second.opts;
			entry["freq"]=(*eIt).second.freq;
			entry["passno"]=(*eIt).second.passno;

			string ruuid = DiskMetaData::Instance().GetDev(EUtils::StringTools::Split((*eIt).second.device,"UUID=").back());
			if( ruuid != "" ) {
				entry["uuid"] = EUtils::StringTools::Split((*eIt).second.device,"UUID=").back();
				entry["device"] = "/dev/" + ruuid; // XXX Assumes they are under /dev
			}
			ret["entries"].append(entry);
		}
	}

	ret["status"]=rval;
	if(arg.size()){
		cout << ret.toStyledString();
	}else{
		cout << Json::FastWriter().write(ret);
	}
	return rval;
}

bool FsTabCmd::do_add(Args& arg){
	// TODO: perhaps validate better against duplicate entries (path,device)
	Json::Value ret=Json::Value(Json::objectValue);
	bool rval=true;

	if(arg.size()!=6){
		rval=false;
		ret["errmsg"]="Wrong amt of args";
	}

	if(rval){
		try{
			if(!S_ISBLK(EUtils::Stat(arg[0]).GetMode())){
				ret["errmsg"]=arg[0]+" doesnt exist or is no block device";
				rval=false;
			}
		}catch(EUtils::EExcept::ENoent e){
			ret["errmsg"]=arg[0]+" doesnt exist or is no block device";
			rval=false;
		}
	}

	if(rval){
		if(!EUtils::Stat::DirExists(arg[1])){
			ret["errmsg"]="Mount directory does not exist";
			rval=false;
		}
	}
	if(rval){
		int freq=atoi(arg[4].c_str());
		int pass=atoi(arg[5].c_str());
		EUtils::FsTab::Entry entry;
		entry.device=arg[0];
		entry.mount=arg[1];
		entry.fstype=arg[2];
		entry.opts=arg[3];
		entry.freq=freq;
		entry.passno=pass;

		EUtils::FsTab tab(FSTAB);
		if(!tab.AddEntry(entry)){
			rval=false;
			ret["errmsg"]="Operation failed";
		}else{
			stringstream ss;
			tab.Write(ss);
			if(!EUtils::FileUtils::Write(FSTAB,ss.str())){
				rval=false;
				ret["errmsg"]="Failed to write new fstab";
			}
		}

	}
	ret["status"]=rval;
	cout << Json::FastWriter().write(ret);
	return rval;
}
bool FsTabCmd::do_add_by_uuid(Args& arg){
	// TODO: perhaps validate better against duplicate entries (path,device)
	Json::Value ret=Json::Value(Json::objectValue);
	bool rval=true;

	if( arg.size() < 2 || arg.size() > 3 ){
		rval=false;
		ret["errmsg"]="Wrong amt of args";
	}

	if(rval){
		try{
			if(!S_ISBLK(EUtils::Stat(arg[0]).GetMode())){
				ret["errmsg"]=arg[0]+" doesnt exist or is no block device";
				rval=false;
			}
		}catch(EUtils::EExcept::ENoent e){
			ret["errmsg"]=arg[0]+" doesnt exist or is no block device: " + e.what();
			rval=false;
		}
	}

	if(rval){
		if(!EUtils::Stat::DirExists(arg[1])){
			ret["errmsg"]="Mount directory does not exist";
			rval=false;
		}
	}
	string uuid = DiskMetaData::Instance().GetUUID(EUtils::StringTools::Split(arg[0],"/").back());

	if(rval){
		if( uuid == "" ) {
			ret["errmsg"]=arg[0]+" didn't have an uuid!";
			rval=false;
		}
	}
	if(rval){


		EUtils::FsTab::Entry entry;
		entry.device = "UUID=" + uuid;
		entry.mount  = arg[1];
		entry.fstype = "auto";
		if( arg.size() == 3 ) {
			entry.opts = arg[2];
		} else {
			entry.opts   = "defaults";
		}
		entry.freq   = 0;
		entry.passno = 0;

		EUtils::FsTab tab(FSTAB);
		if(!tab.AddEntry(entry)){
			rval=false;
			ret["errmsg"]="Operation failed";
		}else{
			stringstream ss;
			tab.Write(ss);
			if(!EUtils::FileUtils::Write(FSTAB,ss.str())){
				rval=false;
				ret["errmsg"]="Failed to write new fstab";
			}
		}

	}
	ret["status"]=rval;
	cout << Json::FastWriter().write(ret);
	return rval;
}

bool FsTabCmd::do_remove(Args& arg){
	Json::Value ret=Json::Value(Json::objectValue);
	bool rval=true;

	if(arg.size()!=1){
		rval=false;
		ret["errmsg"]="Wrong amt of args";
	}

	if(rval){
		EUtils::FsTab tab(FSTAB);
		if(!tab.DelByDevice(arg[0])){
			if(!tab.DelByMount(arg[0])){
				rval=false;
				ret["errmsg"]="Device or mountpoint not found";
			}
		}
		if(rval){
			stringstream ss;
			tab.Write(ss);
			if(!EUtils::FileUtils::Write(FSTAB,ss.str())){
				rval=false;
				ret["errmsg"]="Failed to write new fstab";
			}
		}
	}

	ret["status"]=rval;
	cout << Json::FastWriter().write(ret);
	return rval;
}

bool FsTabCmd::do_check_is_mounted(Args& arg){
	Json::Value ret=Json::Value(Json::objectValue);
	bool rval=true;

	if(arg.size()!=1){
		rval=false;
		ret["errmsg"]="Wrong amt of args";
	}

	if(rval){
		ret["mounted"] = Disks::IsMounted(arg[0]);
	}

	ret["status"]=rval;
	cout << Json::FastWriter().write(ret);
	return rval;
}

bool FsTabCmd::do_mount(Args& arg){
	Json::Value ret=Json::Value(Json::objectValue);
	bool rval=true;

	if(arg.size()!=1){
		rval=false;
		ret["errmsg"]="Wrong amt of args";
	}

	if(rval){
		if(!EUtils::FsTab::Mount(arg[0])){
			string uuid = DiskMetaData::Instance().GetUUID(EUtils::StringTools::Split(arg[0],"/").back());
			if( uuid == "" || !EUtils::FsTab::Mount("UUID=" + uuid ) ) {
				rval=false;
				ret["errmsg"]="Operation failed";
			}
		}
	}

	ret["status"]=rval;
	cout << Json::FastWriter().write(ret);
	return rval;
}

bool FsTabCmd::do_umount(Args& arg){
	Json::Value ret=Json::Value(Json::objectValue);
	bool rval=true;

	if(arg.size()!=1){
		rval=false;
		ret["errmsg"]="Wrong amt of args";
	}

	if(rval){
		if(!EUtils::FsTab::UMount(arg[0])){
			rval=false;
			ret["errmsg"]="Operation failed";
		}
	}

	ret["status"]=rval;
	cout << Json::FastWriter().write(ret);
	return rval;
}


bool FsTabCmd::operator()(Args& arg){
	if(arg.size()==0){
		return false;
	}

	string cmd=arg[0];
	arg.erase(arg.begin());

	if(cmd=="list"){
		return this->do_list(arg);
	}else if(cmd=="add"){
		return this->do_add(arg);
	}else if(cmd=="add_by_uuid"){
		return this->do_add_by_uuid(arg);
	}else if(cmd=="remove"){
		return this->do_remove(arg);
	}else if(cmd=="is_mounted"){
		return this->do_check_is_mounted(arg);
	}else if(cmd=="mount"){
		return this->do_mount(arg);
	}else if(cmd=="umount"){
		return this->do_umount(arg);
	}else{
		return false;
	}

	return true;
}

FsTabCmd::~FsTabCmd(){
}

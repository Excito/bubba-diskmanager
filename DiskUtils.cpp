#include "DiskUtils.h"

#include <libeutils/StringTools.h>
#include <libeutils/FileUtils.h>

DiskMetaData::DiskMetaData(){
	this->get_labels();
	this->get_uuid();
}

DiskMetaData& DiskMetaData::Instance(){
	static DiskMetaData dmd;
	
	return dmd;
}


string DiskMetaData::GetUUID(const string& dev){
	if(this->uuids.find(dev)!=this->uuids.end()){
		return this->uuids[dev];
	}else{
		return "";
	}
}
string DiskMetaData::GetDev(const string& uuid){
	if(this->ruuids.find(uuid)!=this->ruuids.end()){
		return this->ruuids[uuid];
	}else{
		return "";
	}
}

string DiskMetaData::GetLabel(const string& dev){
	if(this->labels.find(dev)!=this->labels.end()){
		return this->labels[dev];
	}else{
		return "";
	}
}


void DiskMetaData::get_labels(){
	list<string> s_label=EUtils::FileUtils::Glob("/dev/disk/by-label/*");
	char lbuf[1024];
	int rd;
	for(list<string>::iterator sIt=s_label.begin();sIt!=s_label.end();sIt++){
		if((rd=readlink((*sIt).c_str(),lbuf,1024))<0){
			continue;
		}
		lbuf[rd]=0;
		this->labels[EUtils::StringTools::Split(lbuf,"/").back()]=EUtils::StringTools::Split((*sIt),"/").back();
	}
}

void DiskMetaData::get_uuid(){
	list<string> s_uuid=EUtils::FileUtils::Glob("/dev/disk/by-uuid/*");
	char lbuf[1024];
	int rd;
	for(list<string>::iterator sIt=s_uuid.begin();sIt!=s_uuid.end();sIt++){
		if((rd=readlink((*sIt).c_str(),lbuf,1024))<0){
			continue;
		}
		lbuf[rd]=0;
		this->uuids[EUtils::StringTools::Split(lbuf,"/").back()]=EUtils::StringTools::Split((*sIt),"/").back();
		this->ruuids[EUtils::StringTools::Split((*sIt),"/").back()]=EUtils::StringTools::Split(lbuf,"/").back();
	}
}


#include "DiskUtils.h"

#include <libeutils/StringTools.h>
#include <libeutils/FileUtils.h>

#ifndef USE_OLD_UDEV
#include <libudev.h>
#endif

DiskMetaData::DiskMetaData(){
#ifdef USE_OLD_UDEV
	this->get_labels();
	this->get_uuid();
#else
	this->init_data();
#endif
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

#ifdef USE_OLD_UDEV
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
#else

static string get_udev_property(struct udev_device *dev, const char* str){
	const char* val;
	val=udev_device_get_property_value(dev,str);

	return val?val:"";
}

void DiskMetaData::init_data(){

	struct udev *udev=udev_new();

	if (!udev) {
		return;
	}

	struct udev_enumerate *enumerate=udev_enumerate_new(udev);
	udev_enumerate_add_match_subsystem(enumerate,"block");
	udev_enumerate_scan_devices(enumerate);

	struct udev_list_entry *devices=udev_enumerate_get_list_entry(enumerate);

	struct udev_list_entry *dev_list_entry;
	udev_list_entry_foreach(dev_list_entry, devices) {
		const char *path=udev_list_entry_get_name(dev_list_entry);

		struct udev_device *dev=udev_device_new_from_syspath(udev,path);
		if (dev) {
			string name=udev_device_get_sysname(dev);
			string uuid=get_udev_property(dev,"ID_FS_UUID");
			string label=get_udev_property(dev,"ID_FS_LABEL");

			this->uuids[name]=uuid;
			this->ruuids[uuid]=name;
			this->labels[name]=label;

			udev_device_unref(dev);
		}
	}

	udev_enumerate_unref(enumerate);

	udev_unref(udev);
}

#endif

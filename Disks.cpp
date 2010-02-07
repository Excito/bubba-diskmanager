#include "Disks.h"

#include <libeutils/StringTools.h>
#include <libeutils/FileUtils.h>
#include <libeutils/Expect.h>
#include <libeutils/Process.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <iostream>
#include <iomanip>
#include <sstream>

#include <stdlib.h>
#include <parted/parted.h>

// TODO: move this to diskmetadata
string Disks::get_partitiontype(const string& dev, int partno){
	string ret="unused";

	PedDevice* device = ped_device_get( dev.c_str() );

	if( ! ped_device_open( device ) )
		throw runtime_error("Failed to open device");

	PedDisk* disk = ped_disk_new( device );
	if( ! disk )
		throw runtime_error("Failed to open partition table");

	PedPartition* part = ped_disk_get_partition( disk, partno);
	if( ! part )
		throw runtime_error("Failed to retrieve partition from partition table");

	if(ped_partition_is_flag_available(part,PED_PARTITION_LVM) && ped_partition_get_flag(part,PED_PARTITION_LVM)){
		ret="pv";
	}else if(ped_partition_is_flag_available(part,PED_PARTITION_RAID) && ped_partition_get_flag(part,PED_PARTITION_RAID)){
		ret="array";
	}else{

	}
	ped_disk_destroy (disk);

	if( ! ped_device_close( device ) )
		throw runtime_error("Failed to close device");

	return ret;
}


void Disks::get_partitions(Device& device, const string& disk){
	device.slaves.clear();
	list<string> parts=EUtils::FileUtils::Glob(disk+"/sd??");
	for(list<string>::iterator lIt=parts.begin();lIt!=parts.end();lIt++){
		Hash val;
		string dev=EUtils::StringTools::Split(*lIt,"/").back();
		val["dev"]="/dev/"+dev;
		val["size"]=EUtils::FileUtils::GetContentAsString((*lIt)+"/size");
		val["type"]="partition";

		int part=atoi(val["dev"].substr(val["dev"].size()-1).c_str());
		string tmpdev=val["dev"].substr(0,val["dev"].size()-1);

		val["usage"]=Disks::get_partitiontype(tmpdev,part);

		val["uuid"]=DiskMetaData::Instance().GetUUID(dev);
		val["label"]=DiskMetaData::Instance().GetLabel(dev);

		device.slaves.push_back(val);
	}

}

string Disks::get_bus(const string& dev){
	string ret="Unknown";

	try{
#ifdef USE_OLD_UDEV
		list<string> env=EUtils::FileUtils::ProcessRead("/usr/bin/udevinfo --query=env --name=/dev/"+dev+" 2>/dev/null", true);
#else
		list<string> env=EUtils::FileUtils::ProcessRead("/sbin/udevadm info --query=env --name=/dev/"+dev+" 2>/dev/null", true);
#endif
		list<string>::iterator eIt;
		for(eIt=env.begin();eIt!=env.end();eIt++){
			if((*eIt).substr(0,6)!="ID_BUS"){
				continue;
			}
			ret=(*eIt).substr(7,string::npos);
			break;
		}
	}catch(std::exception& err){
	}
	return ret;

}

void Disks::get_disks(){

	this->disks.clear();

	list<string> disks=EUtils::FileUtils::Glob("/sys/block/sd?");
	for(list<string>::iterator lIt=disks.begin();lIt!=disks.end();lIt++){
		Device val;
		string dev=EUtils::StringTools::Split(*lIt,"/").back();
		val.attr["type"]="disk";
		val.attr["bus"]=Disks::get_bus(dev);
		val.attr["dev"]="/dev/"+dev;
		val.attr["size"]=EUtils::FileUtils::GetContentAsString((*lIt)+"/size");
		val.attr["model"]=EUtils::StringTools::Trimmed(
			EUtils::FileUtils::GetContentAsString("/sys/block/"+dev+"/device/model")," \t");
		val.attr["vendor"]=EUtils::StringTools::Trimmed(
			EUtils::FileUtils::GetContentAsString("/sys/block/"+dev+"/device/vendor")," \t");
		this->get_partitions(val,*lIt);

		if(val.slaves.size()==0){
			val.attr["usage"]="unused";
		}else{
			val.attr["usage"]="parent";
		}

		val.attr["uuid"]=DiskMetaData::Instance().GetUUID(dev);
		val.attr["label"]=DiskMetaData::Instance().GetLabel(dev);

		this->disks.push_back(val);
	}
}

void Disks::get_mounts(){
	this->mounts.clear();
	list<string> mounts=EUtils::FileUtils::GetContent("/etc/mtab");
	for(list<string>::iterator lIt=mounts.begin();lIt!=mounts.end();lIt++){
		list<string> fields=EUtils::StringTools::Split(*lIt,' ');
		if(fields.size()!=6){
			continue;
		}
		Hash mp;
		list<string>::iterator fIt=fields.begin();
		mp["dev"]=*fIt++;
		mp["path"]=*fIt++;
		mp["fstype"]=*fIt++;
		mp["options"]=*fIt++;
		mp["freq"]=*fIt++;
		mp["passno"]=*fIt++;

		this->mounts.push_back(mp);
	}
}

void Disks::mark_mounted(){

	for(list<Hash>::iterator mIt=this->mounts.begin();mIt!=this->mounts.end();mIt++){
		// Skip all none "physical" devices
		if((*mIt)["dev"][0]!='/'){
			continue;
		}
		for(list<Device>::iterator dIt=this->disks.begin();dIt!=this->disks.end();dIt++){
			if((*dIt).attr["dev"]==(*mIt)["dev"]){
				(*dIt).attr["usage"]="mounted";
				(*dIt).attr["mountpath"]=(*mIt)["path"];
			}else{
				// Check partitions
				for(list<Hash>::iterator pIt=(*dIt).slaves.begin();
					pIt!=(*dIt).slaves.end();pIt++){
					if((*pIt)["dev"]==(*mIt)["dev"]){
						(*pIt)["usage"]="mounted";
						(*pIt)["mountpath"]=(*mIt)["path"];
					}else if(pIt->count("mountpath") == 0){
						(*pIt)["mountpath"]="";
					}
				}
			}
		}
	}

}

// Creates one partition spanning whole device type either LVM or Raid
bool Disks::CreateSimplePart(const string& dev, enum Disks::PartType parttype, const string& label ){

	PedPartitionFlag flag;
	bool set_flag = true;

	switch(parttype){
	case Disks::Raid:
		flag = PED_PARTITION_RAID;
		break;
	case Disks::LVM:
		flag = PED_PARTITION_LVM;
		break;
	case Disks::Raw:
		set_flag = false;
		break;
	default:
		return false;
	}

	PedDevice* device = ped_device_get( dev.c_str() );
	if( ! ped_device_open( device ) )
		throw disk_error("Failed to open device");

	PedDiskType* type = ped_disk_type_get( "gpt" );

	PedDisk* disk = ped_disk_new_fresh( device, type );
	if( !disk )
		throw disk_error("Failed to create new partition table");

	PedConstraint* constraint = ped_constraint_any( device );
	PedGeometry* geom = ped_constraint_solve_max( constraint );

	PedPartition* part = ped_partition_new( disk, PED_PARTITION_NORMAL, NULL, geom->start, geom->end );
	if( !part )
		throw disk_error("Failed to create new partition");

	ped_exception_fetch_all();

	if(!ped_disk_add_partition( disk, part, constraint )){
		ped_exception_leave_all();
		throw disk_error("Failed to add the new partition to the partition table");
	} else {
		ped_exception_leave_all();
	}

	ped_exception_catch();
	ped_partition_set_name( part, label.c_str() );

	if (ped_partition_is_flag_available( part, PED_PARTITION_LBA ) )
		ped_partition_set_flag( part, PED_PARTITION_LBA, 1 );

	if( set_flag ) {
		ped_partition_set_flag( part, flag, 1 );
	}

	if (!ped_disk_commit_to_dev( disk ) )
		throw disk_error("Failed to write new partition table to disk");

	if (!ped_disk_commit_to_os( disk ) )
		throw disk_error("Failed to inform the OS about the changes");

	ped_geometry_destroy( geom );
	ped_constraint_destroy( constraint );
	ped_disk_destroy( disk );	

	if( ! ped_device_close( device ) )
		throw disk_error("Failed to close device");

	return true;
}

bool Disks::SetPartitionType(const string& dev, const string& partition, enum Disks::PartType parttype){
	PedDevice* device = ped_device_get( dev.c_str() );

	if( ! ped_device_open( device ) )
		throw disk_error("Failed to open device");

	PedDisk* disk = ped_disk_new( device );
	if( ! disk ) 
		throw disk_error("Failed to open partition table");

	PedPartition* part = ped_disk_get_partition( disk, atoi(partition.c_str()) );
	if( ! part )
		throw disk_error("Failed to retrieve partition from partition table");

	PedPartitionFlag flag;

	switch(parttype){
	case Disks::Raid:
		flag = PED_PARTITION_RAID;
		break;
	case Disks::LVM:
		flag = PED_PARTITION_LVM;
		break;
	default:
		return false;
	}
	ped_partition_set_flag (part, flag, 1);

	if (!ped_disk_commit_to_dev( disk ) )
		throw disk_error("Failed to write new partition table to disk");

	if (!ped_disk_commit_to_os( disk ) )
		throw disk_error("Failed to inform the OS about the changes");

	ped_disk_destroy (disk);	

	if( ! ped_device_close( device ) )
		throw disk_error("Failed to close device");

	return true;
}

bool Disks::IsMounted(const string& dev){
	list<string> mounts=EUtils::FileUtils::GetContent("/proc/mounts");
	for(list<string>::iterator lIt=mounts.begin();lIt!=mounts.end();lIt++){
		list<string> fields=EUtils::StringTools::Split(*lIt,' ');
		if(fields.size()!=6){
			continue;
		}
		list<string>::iterator fIt = fields.begin();
		string mtab_dev = EUtils::StringTools::Trimmed(*fIt++, "\n\t ");
		string mtab_path =  EUtils::StringTools::Trimmed(*fIt++, "\n\t ");
		if( mtab_dev == dev || mtab_path == dev ) {
			return true;
		}
	}
	return false;
}

bool Disks::Probe( const string& dev ) {
	PedDiskType*	disk_type;
	PedDisk*	disk;
	PedDevice *device = ped_device_get( dev.c_str() );

	disk_type = ped_disk_probe( device );
	if (!disk_type || !strcmp (disk_type->name, "loop"))
		return true;

	disk = ped_disk_new( device );
	
	if(!disk) 
		throw disk_error("Failed to open partition table");

	if (!ped_disk_commit_to_os( disk ) )
		throw disk_error("Failed to commit to the OS");

	ped_disk_destroy( disk );

	return true;
}

bool Disks::ExtendPartition(const string& dev){

	const char* cmd[] = {
		"/sbin/resize2fs",
		"-f",
		dev.c_str(),
		NULL
	};

	EUtils::Process p;
	if( p.call( cmd ) != 0 ) {
		return false;
	}
	return true;

}

bool Disks::FormatPartition(const string& dev, const string& fstype){

	this->UpdateStatus("0.00");

	EUtils::Expect e(0);

	e.setTimeout(-1);

	list<string> args;
	args.push_back("mkfs");
	args.push_back("-t");
	args.push_back(fstype);
	args.push_back(dev);

	try{
		int ret;

		e.spawn("/sbin/mkfs",args);

		vector<EUtils::Expect::ExpVal> ev;
		ev.push_back(make_pair(EUtils::Expect::ExpGlob,"mke2fs *"));
		ev.push_back(make_pair(EUtils::Expect::ExpGlob,"mkfs.vfat *"));
		ev.push_back(make_pair(EUtils::Expect::ExpGlob,"mkfs.msdos *"));

		if ((ret=e.expect(ev)) < 0) {
			return false;
		}

		if(ret>1){
			//Todo: vfat or msdos. We cant get progress of those(?)
			if(wait(NULL)<0){
				return false;
			}

			this->UpdateStatus("100.00");
			return true;

		}

		if(e.expect("Superblock backups stored on blocks:")<0){
			return false;
		}

		if(e.expect("Writing inode tables:")<0){
			return false;
		}

		ev.clear();
		ev.push_back(make_pair(EUtils::Expect::ExpGlob,"*/*"));
		ev.push_back(make_pair(EUtils::Expect::ExpExact,"done"));

		do{
			if ((ret=e.expect(ev)) < 0) {
				return false;
			}
			if(ret==1){
				stringstream status;
				list<string> prog=EUtils::StringTools::Split(EUtils::StringTools::Trimmed(e.match(),"\x08"),'/');
				double done=( static_cast<double>(atoi(prog.front().c_str())+1) / static_cast<double>(atoi(prog.back().c_str())) ) *100;
				status <<setiosflags(ios::fixed) << setprecision(2)<<done;
				this->UpdateStatus(status.str());
			}
		}while(ret==1);


		if(e.expect("Writing superblocks and filesystem accounting information:")<0){
			return false;
		}

		if(e.expect("Use tune2fs -c or -i to override.")<0){
			return false;
		}

		if(wait(NULL)<0){
			return false;
		}
		
		const char* cmd[] = {
			"/sbin/tune2fs",
			"-c", "0",
			"-i", "0", 
			dev.c_str(),
			NULL
		};

		EUtils::Process p;
		if( p.call( cmd ) != 0 ) {
			return false;
		}

	}catch(exception& e){
		return false;
	}

	return true;
}

list<Hash> Disks::GetDisks(){
	list<Hash> res;
	for(list<Device>::iterator dIt=this->disks.begin();dIt!=this->disks.end();dIt++){
		if((*dIt).attr["usage"]=="unused" || (*dIt).attr["usage"]=="mounted" ){
			res.push_back((*dIt).attr);
		}else{
			for(list<Hash>::iterator pIt=(*dIt).slaves.begin();pIt!=(*dIt).slaves.end();pIt++){
				Hash dev=(*pIt);
				dev["model"]=(*dIt).attr["model"];
				dev["vendor"]=(*dIt).attr["vendor"];
				res.push_back(dev);
			}
		}
	}
	return res;
}


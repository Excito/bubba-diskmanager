
#include <sstream>
#include <stdlib.h>

#include <iostream>
#include <fstream>

#include "RaidDevs.h"

#include <libeutils/Regex.h>
#include <libeutils/StringTools.h>
#include <libeutils/FileUtils.h>
#include <libeutils/EExcept.h>
#include <libeutils/Process.h>

using namespace EUtils;

//TODO: refactor this into eUtils :(
static int do_call(const string& cmd){
	int ret=system(cmd.c_str());
	if(ret<0){
		return ret;
	}
	return WEXITSTATUS(ret);
}

void RaidDevs::Mark(Disks& disks){
	// Forech md
	for(list<Device>::iterator mIt=mds.begin();mIt!=mds.end();mIt++){
		// Foreach slave
		for(list<Hash>::iterator sIt=(*mIt).slaves.begin();sIt!=(*mIt).slaves.end();sIt++){
			//Foreach disk
			for(list<Device>::iterator dIt=disks.disks.begin();dIt!=disks.disks.end();dIt++){
				if((*sIt)["dev"]==(*dIt).attr["dev"]){
					(*dIt).attr["usage"]="array";
					(*dIt).attr["md"]=(*mIt).attr["dev"];
				}else{
					// Foreach partition
					for(list<Hash>::iterator pIt=(*dIt).slaves.begin();
						pIt!=(*dIt).slaves.end();pIt++){

						if((*sIt)["dev"]==(*pIt)["dev"]){
							(*pIt)["usage"]="array";
							(*pIt)["md"]=(*mIt).attr["dev"];
						}

					}
				}
			}
		}
	}
}

list<Device> RaidDevs::GetMDs(){
	return this->mds;
}

void RaidDevs::get_mds(){
	list<string> mds=EUtils::FileUtils::Glob("/sys/block/md?");
	for(list<string>::iterator sIt=mds.begin();sIt!=mds.end();sIt++){

		Device val;

		this->get_mdslaves(val,*sIt);
		if(val.slaves.size()==0){
			// No slaves found this is most likely a inactive array, dont add
#if 1
			continue;
#endif
		}
		string dev=EUtils::StringTools::Split(*sIt,"/").back();
		val.attr["dev"]="/dev/"+dev;
		val.attr["size"]=EUtils::FileUtils::GetContentAsString(*sIt+"/size");
		val.attr["type"]="array";
		val.attr["level"]=EUtils::FileUtils::GetContentAsString(*sIt+"/md/level");
		val.attr["state"]=EUtils::FileUtils::GetContentAsString(*sIt+"/md/array_state");
		val.attr["chunksize"]=EUtils::FileUtils::GetContentAsString(*sIt+"/md/chunk_size");
		val.attr["disks"]=EUtils::FileUtils::GetContentAsString(*sIt+"/md/raid_disks");
		val.attr["sync"]=EUtils::FileUtils::GetContentAsString(*sIt+"/md/sync_action");
		val.attr["sync_completed"]=EUtils::FileUtils::GetContentAsString(*sIt+"/md/sync_completed");
		val.attr["sync_speed"]=EUtils::FileUtils::GetContentAsString(*sIt+"/md/sync_speed");
		val.attr["degraded"]=EUtils::FileUtils::GetContentAsString(*sIt+"/md/degraded");
		val.attr["uuid"]=DiskMetaData::Instance().GetUUID(dev);
		val.attr["label"]=DiskMetaData::Instance().GetLabel(dev);


		this->mds.push_back(val);

	}
}

void RaidDevs::get_mdslaves(Device& device, const string& path){
	list<string> slaves=EUtils::FileUtils::Glob(path+"/md/dev-*");
	for(list<string>::iterator sIt=slaves.begin();sIt!=slaves.end();sIt++){
		Hash val;
		string dev=EUtils::StringTools::Split(*sIt,"/dev-").back();
		val["dev"]="/dev/"+dev;
		val["size"]=EUtils::FileUtils::GetContentAsString(*sIt+"/size");
		val["state"]=EUtils::FileUtils::GetContentAsString(path+"/md/dev-"+dev+"/state");
		val["slot"]=EUtils::FileUtils::GetContentAsString(path+"/md/dev-"+dev+"/slot");
		val["errors"]=EUtils::FileUtils::GetContentAsString(path+"/md/dev-"+dev+"/errors");
		val["type"]=val["slot"]=="none"?"spare":"arrayslave";
		device.slaves.push_back(val);
	}
}

bool RaidDevs::md_exists(const string& dev){
	for(list<Device>::iterator dIt=this->mds.begin();dIt!=this->mds.end();dIt++){
		if((*dIt).attr["dev"]==dev){
			return true;
		}
	}
	return false;
}

// Remove all DEVICES entries from mdadm.conf located at path
static bool remove_devices(const string& path){
	list<string> lread=FileUtils::GetContent(path);
	list<string> lwrite;
	bool match=false;
	for(list<string>::iterator sIt=lread.begin();sIt!=lread.end();sIt++){
		if((*sIt).compare(0,6,"ARRAY ")==0){
			match=true;
			continue;
		}
		if(match && ((*sIt)[0]==' ' || (*sIt)[0]=='\t')){
			// this is a continuation of array line started before
			continue;
		}
		match=false;
		lwrite.push_back((*sIt)+"\n");
	}

	return FileUtils::Write(path,lwrite);
}

bool RaidDevs::CreateMD(RaidDevs::types type,int ndisks, int spares, list<string>& devs, string devpath){
	if(ndisks<=0){
		return false;
	}
	if(spares<0){
		return false;
	}

	for(list<string>::iterator sIt=devs.begin();sIt!=devs.end();sIt++){
		try{
			Stat s(*sIt);
			if(!S_ISBLK(s.GetMode())){
				return false;
			}
		}catch(EUtils::EExcept::ENoent err){
			return false;
		}
	}

	if(devpath==""){
		devpath = this->GetNextMD();
		if(devpath==""){
			return false;
		}
	}
	if(this->md_exists(devpath)){
		return false;
	}
	{
		vector<const char*> cmd;
		cmd.push_back( MDADM );
		cmd.push_back( "-e" );
		cmd.push_back( "0.90" );
		cmd.push_back( "--create" );
		cmd.push_back( "--run" );
		cmd.push_back( "--force" );
		cmd.push_back( "--assume-clean" );
		cmd.push_back( devpath.c_str() );
		cmd.push_back( "--level" );
		{
			std::stringstream ss;
			std::string s;
			ss << type;
			s = ss.str();
			cmd.push_back( s.c_str() );
		}
		cmd.push_back( "--raid-devices" );
		{
			std::stringstream ss;
			std::string s;
			ss << ndisks;
			s = ss.str();
			cmd.push_back( s.c_str() );
		}
		if(spares>0){
			cmd.push_back( "--spare-devices" );
			std::stringstream ss;
			std::string s;
			ss << spares;
			s = ss.str();
			cmd.push_back( s.c_str() );
		}

		for(list<string>::iterator sIt=devs.begin();sIt!=devs.end();sIt++){
			cmd.push_back( sIt->c_str() );
		}
		cmd.push_back( NULL );
		EUtils::Process p;

		if( p.call( &cmd[0] ) != 0 ) {
			return false;
		}
	}

	if(!remove_devices(MDCONF)){
		return false;
	}
	const char* cmd[] = {
		MDADM,
		"--examine",
		"--scan",
		"--config=partitions",
		NULL
	};

	EUtils::Process p;
	if( p.call( cmd ) != 0 ) {
		return false;
	}

	fstream mdconf( MDCONF, fstream::out | fstream::app );

	mdconf << p.pout->str() << endl;

	mdconf.close();

	return true;
}

bool RaidDevs::StopMD(const string& dev){

	const char* cmd[] = {
		MDADM,
		"--stop",
		dev.c_str(),
		NULL
	};

	EUtils::Process p;
	if( p.call( cmd ) != 0 ) {
		return false;
	}

	return true;
}

string RaidDevs::GetNextMD(){

	string path;
	int i=0;
	do{
		stringstream ss;
		ss<<"/dev/md"<<i;
		path=ss.str();
	}while(this->md_exists(path)&&(++i<10));

	return path;
}

bool RaidDevs::AssembleMD(list<string>& devs){
	vector<const char*> cmd;
	cmd.push_back( MDADM );
	cmd.push_back( "--assemble" );
	for(list<string>::iterator sIt=devs.begin();sIt!=devs.end();sIt++){
		cmd.push_back( sIt->c_str() );
	}
	cmd.push_back( "--run" );
	cmd.push_back( "--force" );
	cmd.push_back( NULL );
	EUtils::Process p;

	if( p.call( &cmd[0] ) != 0 ) {
		// ignore, as an half array can still be built
		//return false;
	}

	return true;
}

// TODO fix
bool RaidDevs::DestroyMD(const string& dev){
	// Make sure device is stopped
	this->StopMD(dev);

	
	if(!remove_devices(MDCONF)){
		return false;
	}

	if(do_call(MDADM " --examine --scan --config=partition >>" MDCONF)!=0){
		return false;
	}

	return true;
}

bool RaidDevs::FailDisk(const string& md, const string& disk){
	
	const char* cmd[] = {
		MDADM,
		"--manage",
		md.c_str(),
		"--fail",
		disk.c_str(),
		NULL
	};

	EUtils::Process p;
	if( p.call( cmd ) != 0 ) {
		return false;
	}

	return true;
}

bool RaidDevs::RemoveDisk(const string& md, const string& disk){

	const char* cmd[] = {
		MDADM,
		"--manage",
		md.c_str(),
		"--remove",
		disk.c_str(),
		NULL
	};

	EUtils::Process p;
	if( p.call( cmd ) != 0 ) {
		return false;
	}

	return true;
}

bool RaidDevs::AddDisk(const string& md, const string& disk){

	const char* cmd[] = {
		MDADM,
		"--manage",
		md.c_str(),
		"--add",
		disk.c_str(),
		NULL
	};

	EUtils::Process p;
	if( p.call( cmd ) != 0 ) {
		return false;
	}

	return true;
}

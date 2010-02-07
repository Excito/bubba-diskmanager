
#include "LVM.h"

#include <libeutils/StringTools.h>
#include <libeutils/FileUtils.h>
#include <libeutils/Process.h>
#include <libeutils/FsTab.h>
#include <iostream>
#include <sstream>
#include <vector>

#include <stdlib.h>

void LVM::Mark(Disks& disks){
	// Forech vg
	for(list<Device>::iterator vIt=vgs.begin();vIt!=vgs.end();vIt++){
		// Foreach pv
		for(list<Hash>::iterator pIt=(*vIt).slaves.begin();pIt!=(*vIt).slaves.end();pIt++){
			//Foreach disk
			for(list<Device>::iterator dIt=disks.disks.begin();dIt!=disks.disks.end();dIt++){
				if((*pIt)["dev"]==(*dIt).attr["dev"]){
					(*dIt).attr["usage"]="pv";
					(*dIt).attr["vgroup"]=(*pIt)["group"];
				}else{
					// Foreach partition
					for(list<Hash>::iterator dpIt=(*dIt).slaves.begin();
						dpIt!=(*dIt).slaves.end();dpIt++){

						if((*pIt)["dev"]==(*dpIt)["dev"]){
							(*dpIt)["usage"]="pv";
							(*dpIt)["vgroup"]=(*pIt)["group"];
						}

					}
				}
			}
		}
	}
}

void LVM::get_pvs(Device& vg){
	//list<string> pvs=EUtils::FileUtils::ProcessRead("pvs --separator=\":\" --noheadings --nosuffix --units b --options=\"pv_fmt,pv_uuid,pv_name,vg_name,pv_size,dev_size,pv_free,pv_used,pv_attr,pv_pe_count,pv_pe_alloc_count\" 2> /dev/null",true);
	const char* cmd[] = {
		"/sbin/pvs",
		"--separator",":",
		"--noheadings",
		"--nosuffix",
		"--units","b",
		"--options","pv_fmt,pv_uuid,pv_name,vg_name,pv_size,dev_size,pv_free,pv_used,pv_attr,pv_pe_count,pv_pe_alloc_count",
		NULL
	};

	list<string> pvs;
	{
		EUtils::Process p;
		p.call( cmd );
		string s;
		while( getline( *p.pout, s ) ) {
			s = EUtils::StringTools::Trimmed(s, "\n\t ");
			if( s == "" ) continue;
			pvs.push_back( s );
		}
	}
	for(list<string>::iterator sIt=pvs.begin();sIt!=pvs.end();sIt++){
		list<string> vals = EUtils::StringTools::Split(*sIt,':');

		Hash val;
		list<string>::iterator valIt=vals.begin();
		val["format"]=*valIt++;
		val["uuid"]=*valIt++;
		val["dev"]=*valIt++;
		val["group"]=*valIt++;
		val["size"]=*valIt++;
		val["devsize"]=*valIt++;
		val["free"]=*valIt++;
		val["used"]=*valIt++;
		val["attr"]=*valIt++;
		val["pe_count"]=*valIt++;
		val["pe_alloc"]=*valIt++;
		val["type"]="physicalvolume";
		if(val["group"]==vg.attr["name"]){
			vg.slaves.push_back(val);
		}
	}
}

void LVM::get_vgs(void){
	//list<string> vgs=EUtils::FileUtils::ProcessRead("vgs --separator=':' --noheadings --nosuffix --units b --options='vg_fmt,vg_uuid,vg_name,vg_attr,vg_size,vg_free,vg_sysid,vg_extent_size,vg_extent_count,vg_free_count,max_lv,max_pv,pv_count,lv_count,snap_count,vg_seqno' 2> /dev/null",true);
	const char* cmd[] = {
		"/sbin/vgs",
		"--separator",":",
		"--noheadings",
		"--nosuffix",
		"--units","b",
		"--options","vg_fmt,vg_uuid,vg_name,vg_attr,vg_size,vg_free,vg_sysid,vg_extent_size,vg_extent_count,vg_free_count,max_lv,max_pv,pv_count,lv_count,snap_count,vg_seqno",
		NULL
	};
	list<string> vgs;
	{
		EUtils::Process p;
		p.call( cmd );
		string s;
		while( getline( *p.pout, s ) ) {
			s = EUtils::StringTools::Trimmed(s, "\n\t ");
			if( s == "" ) continue;
			vgs.push_back( s );
		}
	}

	for(list<string>::iterator sIt=vgs.begin();sIt!=vgs.end();sIt++){
		list<string> vals = EUtils::StringTools::Split(*sIt,':');

		Device val;
		list<string>::iterator valIt=vals.begin();
		val.attr["format"]=*valIt++;
		val.attr["uuid"]=*valIt++;
		val.attr["name"]=*valIt++;
		val.attr["attr"]=*valIt++;
		val.attr["size"]=*valIt++;
		val.attr["free"]=*valIt++;
		val.attr["sysid"]=*valIt++;
		val.attr["extent_size"]=*valIt++;
		val.attr["extent_count"]=*valIt++;
		val.attr["free_count"]=*valIt++;
		val.attr["max_lv"]=*valIt++;
		val.attr["max_pv"]=*valIt++;
		val.attr["pv_count"]=*valIt++;
		val.attr["lv_count"]=*valIt++;
		val.attr["snap_count"]=*valIt++;
		val.attr["vg_seqno"]=*valIt++;
		val.attr["type"]="volumegroup";
		get_pvs(val);
		get_lvs(val);
		this->vgs.push_back(val);
	}

}

void LVM::get_lvs(Device &vg){
	const char* cmd[] = {
		"/sbin/lvs",
		"--separator",":",
		"--noheadings",
		"--nosuffix",
		"--units","b",
		"--options","lv_uuid,lv_name,lv_attr,lv_major,lv_minor,lv_size,vg_name,seg_count",
		NULL
	};
	list<string> lvs;
	{
		EUtils::Process p;
		p.call( cmd );
		string s;
		while( getline( *p.pout, s ) ) {
			s = EUtils::StringTools::Trimmed(s, "\n\t ");
			if( s == "" ) continue;
			lvs.push_back( s );
		}
	}
	for(list<string>::iterator sIt=lvs.begin();sIt!=lvs.end();sIt++){
		list<string> vals = EUtils::StringTools::Split(*sIt,':');
	
		Hash val;
		list<string>::iterator valIt=vals.begin();
		val["uuid"]=*valIt++;
		val["name"]=*valIt++;
		val["attr"]=*valIt++;
		val["major"]=*valIt++;
		val["minor"]=*valIt++;
		val["size"]=*valIt++;
		val["group"]=*valIt++;
		val["segments"]=*valIt++;
		val["type"]="logicalvolume";
		if(val["group"]==vg.attr["name"]){

			EUtils::FsTab tab("/etc/fstab");
			map<string,EUtils::FsTab::Entry> entries=tab.GetEntries();
			string fstab_uuid("UUID=");
			fstab_uuid += val["uuid"];
			string fstab_dev_mapper("/dev/mapper/");
			fstab_dev_mapper += val["group"] + "-" + val["name"];
			string fstab_dev_dev("/dev/");
			fstab_dev_dev += val["group"] + "/" + val["name"];

			map<string,EUtils::FsTab::Entry>::iterator eIt1 = entries.find( fstab_uuid );
			map<string,EUtils::FsTab::Entry>::iterator eIt2 = entries.find( fstab_dev_mapper );
			map<string,EUtils::FsTab::Entry>::iterator eIt3 = entries.find( fstab_dev_dev );
			if( eIt1 != entries.end() ) {
				val["mountpath"] = (*eIt1).second.mount;
			} else if( eIt2 != entries.end() ) {
				val["mountpath"] = (*eIt2).second.mount;
			} else if( eIt3 != entries.end() ) {
				val["mountpath"] = (*eIt3).second.mount;
			} else {
				val["mountpath"] = "";
			}
	
			string devpath( "/dev/" + val["group"] +  "/" + val["name"] );
			const char* cmd[] = {
				"/sbin/lvs",
				"--noheadings",
				"--nosuffix",
				"--units","b",
				"--options","devices",
				devpath.c_str(),
				NULL
			};
			stringlist resultpath;
			{
				EUtils::Process p;
				p.call( cmd );
				string s;
				while( getline( *p.pout, s ) ) {
					s = EUtils::StringTools::Trimmed(s, "\n\t ");
					if( s == "" ) continue;
					resultpath.push_back( s );
				}
			}

			vg.logics_devices[devpath] = resultpath;

			vg.logics.push_back(val);
		}
	}

}
list<Device> LVM::GetDevices(){
	return this->vgs;
}

bool LVM::CreatePV(const string& dev){

	const char* cmd[] = {
		"/sbin/pvcreate",
		dev.c_str(),
		NULL
	};

	EUtils::Process p;
	if( p.call( cmd ) != 0 ) {
		return false;
	}
	return true;
}

bool LVM::RemovePV(const string& dev){
	const char* cmd[] = {
		"/sbin/pvremove",
		dev.c_str(),
		NULL
	};

	EUtils::Process p;
	if( p.call( cmd ) != 0 ) {
		return false;
	}
	return true;
}

bool LVM::RemoveVG(const string& vgname){

	const char* cmd[] = {
		"/sbin/vgremove",
		vgname.c_str(),
		NULL
	};

	EUtils::Process p;
	if( p.call( cmd ) != 0 ) {
		return false;
	}
	return true;
}

bool LVM::ExtendVG(const string& vgname, const string& device){

	const char* cmd[] = {
		"/sbin/vgextend",
		vgname.c_str(),
		device.c_str(),
		NULL
	};

	EUtils::Process p;
	if( p.call( cmd ) != 0 ) {
		return false;
	}
	return true;
}



bool LVM::CreateVG(const string& vgroup, list<string> devices){

	if(devices.size()==0){
		return false;
	}

	if(vgroup==""){
		return false;
	}
	vector<const char*> cmd;
	cmd.push_back( "vgcreate" );
	cmd.push_back( vgroup.c_str() );

	for(list<string>::iterator sIt=devices.begin();sIt!=devices.end();sIt++){
		cmd.push_back( sIt->c_str() );
	}
	cmd.push_back( NULL );

	EUtils::Process p;
	if( p.call( &cmd[0] ) != 0 ) {
		return false;
	}
	return true;

}

bool LVM::CreateLV(const string& lvname, const string& vgname){
	if(lvname==""){
		throw empty_path("lvname is empty");
	}
	if(vgname==""){
		throw empty_path("vgname is empty");
	}
	{
		const char* cmd[] = {
			"/sbin/vgs",
			vgname.c_str(),
			NULL
		};

		EUtils::Process p;
		if( p.call( cmd ) != 0 ) {
			throw volumegroup_not_found();
		}
	}
	string size;
	{
		const char* cmd[] = {
			"/sbin/vgs",
			"--noheadings",
			"--nosuffix",
			"--units","b",
			"--options","vg_free_count",
			vgname.c_str(),
			NULL
		};
		EUtils::Process p;
		p.call( cmd );

		size = EUtils::StringTools::Trimmed( p.pout->str(),"\t\n ");
	}
	const char* cmd[] = {
		"/sbin/lvcreate",
		"--extents",
		size.c_str(),
		"--name",
		lvname.c_str(),
		vgname.c_str(),
		NULL
	};
	EUtils::Process p;
	if( p.call( cmd ) != 0 ) {
		return false;
	}
	return true;
}

bool LVM::RemoveLV(const string& lvpath){
	if(lvpath==""){
		throw empty_path("lvpath is empty");
	}
	const char* cmd[] = {
		"/sbin/lvremove",
		"--force",
		lvpath.c_str(),
		NULL
	};

	EUtils::Process p;
	if( p.call( cmd ) != 0 ) {
		return false;
	}
	return true;
}

bool LVM::ExtendLV(const string& lvpath){
	if(lvpath==""){
		throw empty_path("lvpath is empty");
	}
	string size;
	{
		const char* cmd[] = {
			"/sbin/lvs",
			"--noheadings",
			"--nosuffix",
			"--units","m",
			"--options","vg_free",
			lvpath.c_str(),
			NULL
		};
		EUtils::Process p;
		p.call( cmd );

		size = EUtils::StringTools::Trimmed( p.pout->str(),"\t\n ");
		if( size == "0" ) {
			throw empty_size();
		}
	}

	const char* cmd[] = {
		"/sbin/lvextend",
		"--size",
		string("+").append(size).append("M ").c_str(),
		lvpath.c_str(),
		NULL
	};
	EUtils::Process p;
	if( p.call( cmd ) != 0 ) {
		return false;
	}
	return true;

}

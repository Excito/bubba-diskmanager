/*
 * UserMount.cpp
 *
 *  Created on: Aug 14, 2009
 *      Author: tor
 */

#include "UserMount.h"

#include <sstream>

#include <libeutils/FileUtils.h>
#include <libeutils/UserGroups.h>
#include <libeutils/EExcept.h>


UserMount::UserMount(const string& desc):Cmd(desc){}

bool UserMount::operator()(Args& arg){
	if(arg.size()<2 || arg.size()>3){
		return false;
	}

	if(!S_ISBLK(EUtils::Stat::Stat(arg[0]).GetMode())){
		return false;
	}

	if(!EUtils::Stat::DirExists(arg[1])){
		return false;
	}

	string fstype="";
	if(arg.size()==3){
		fstype="-t "+arg[2];
	}

	gid_t gid=EUtils::Group::GroupToGID("users");

	// Try "win"-fs mount
	try{
		stringstream cmd;
		cmd << "/bin/mount " << fstype<<" -ogid=" ;
		cmd << gid << ",umask=0 "<<arg[0]<<" "<<arg[1]+">/dev/null 2>/dev/null";
		EUtils::FileUtils::ProcessRead(cmd.str());
	}catch(std::runtime_error e){
		// Failed lets try normal mount
		try{
			stringstream cmd;
			cmd << "/bin/mount " << fstype;
			cmd << " "<<arg[0]<<" "<<arg[1]+">/dev/null 2> /dev/null";
			EUtils::FileUtils::ProcessRead(cmd.str());
		}catch(std::runtime_error e){
			return false;
		}
	}


	return true;
}


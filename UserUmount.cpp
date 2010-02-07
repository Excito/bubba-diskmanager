/*
 * UserUmount.cpp
 *
 *  Created on: Aug 14, 2009
 *      Author: tor
 */

#include "UserUmount.h"

#include <libeutils/FileUtils.h>
#include <libeutils/EExcept.h>

#include <sstream>

using namespace std;

UserUmount::UserUmount(const string& desc):Cmd(desc){}

bool UserUmount::operator()(Args& arg){
	if(arg.size()!=1){
		return false;
	}

	if(!EUtils::Stat::DirExists(arg[0])){
		return false;
	}

	// Failed lets try normal mount
	//Todo: Where did this one come from...
	try{
		stringstream cmd;
		cmd << "/bin/umount "<<arg[0];
		EUtils::FileUtils::ProcessRead(cmd.str());
	}catch(std::runtime_error& e){
		return false;
	}
	return true;
}

/*
 * CmdDevs.cpp
 *
 *  Created on: Aug 14, 2009
 *      Author: tor
 */

#include "CmdDevs.h"

#include <libeutils/json/json.h>

#include "Disks.h"
#include "LVM.h"
#include "RaidDevs.h"
#include "Utils.h"

CmdDevs::CmdDevs(const string& desc):Cmd(desc){}
bool CmdDevs::operator()(Args& arg){
	Disks disks;
	LVM lvm;
	lvm.Mark(disks);
	RaidDevs rd;
	rd.Mark(disks);
	list<Hash> devs=disks.GetDisks();
	Json::Value ret(Json::arrayValue);
	for(list<Hash>::iterator dIt=devs.begin();dIt!=devs.end();dIt++){
		ret.append(fromHash(*dIt));
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

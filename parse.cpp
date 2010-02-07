
#include <libeutils/SysvShm.h>
#include <libeutils/FileUtils.h>

#include "CmdApp.h"

#include "FsTabCmd.h"
#include "CmdDevs.h"
#include "UserUmount.h"
#include "UserMount.h"
#include "CmdMD.h"
#include "CmdLV.h"
#include "DiskCmd.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdio>

#define SHMFILE "/tmp/dmgshm"

using namespace std;
using namespace EUtils;

static SysvShm* shm;
static char* statbuf;

static void printstatus(const string& status){
	sprintf(statbuf,"%s",status.c_str());
}

static bool setup_shm(){
	if(Stat::FileExists(SHMFILE)){
		// Make sure we have right privs
		if(chmod(SHMFILE,0644)<0){
			return false;
		}
	}else{
		if(!FileUtils::Write(SHMFILE,"",0644)){
			return false;
		}

	}

	shm=new SysvShm(4096,SHMFILE, getpid() );
	statbuf=static_cast<char*>(shm->Value());
	return true;
}

static void remove_shm(){
	shm->Remove();
	delete shm;
}

int main(int argc, char** argv){

	if(!setup_shm()){
		return 1;
	}

	CmdApp c(argv[0]);

	c.AddCmd("md",new CmdMD());
	c.AddCmd("lv",new CmdLV());
	c.AddCmd("list_devices",new CmdDevs("List devices in system"));

	DiskCmd* disk=new DiskCmd();
	disk->StatusChanged.connect(sigc::ptr_fun(printstatus));
	c.AddCmd("disk",disk);

	c.AddCmd("fstab",new FsTabCmd());
	c.AddCmd("user_mount",new UserMount("Mount volume, 'device' 'mountpoint' [fstype]"));
	c.AddCmd("user_umount",new UserUmount("Umount volume, 'device' or 'path'"));
	if(!c.Run(argc,argv)){
		remove_shm();
		return 1;
	}

	remove_shm();
	return 0;
}



#ifndef RAIDDEVS_H
#define RAIDDEVS_H

#include "DiskUtils.h"
#include "Disks.h"

#include <list>
#include <string>

#define MDADM	"/sbin/mdadm"
#define MDCONF	"/etc/mdadm/mdadm.conf"

using namespace std;

class RaidDevs: public StatusNotifier{
public:
	enum types{
		RAID0=0,
		RAID1=1
	};

	RaidDevs():disks(){
		this->get_mds();
	};

	void Mark(Disks& disks);
	list<Device> GetMDs();
	bool CreateMD(RaidDevs::types type,int disks, int spares, list<string>& devs, string devpath="");
	bool AssembleMD(list<string>& devs);
	string GetNextMD();
	bool DestroyMD(const string& dev);
	bool StopMD(const string& dev);
	bool FailDisk(const string& md, const string& disk);
	bool RemoveDisk(const string& md, const string& disk);
	bool AddDisk(const string& md, const string& disk);
	virtual ~RaidDevs(){};

private:
	// Found arrays
	list<Device> mds;
	// Physical disks in system
	list<Device> disks;

	bool md_exists(const string& dev);
	void get_mdslaves(Device& dev,const string& path);
	void get_mds();
};

#endif

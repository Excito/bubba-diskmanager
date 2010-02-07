#ifndef DISKS_H
#define DISKS_H

#include "DiskUtils.h"
#include "StatusNotifier.h"
#include <list>
#include <string>
#include <stdexcept>

using namespace std;

class LVM;
class RaidDevs;

class Disks: public StatusNotifier{
public:
	class disk_error: public std::runtime_error {
	public:
		explicit disk_error(const std::string &s): std::runtime_error(s) {}
		explicit disk_error(): std::runtime_error("Unknown runtime error") {}
	};
	enum PartType{
		Raw=0x0,
		LVM=0x8e,
		Raid=0xfd
	};
	Disks(){
		this->get_disks();
		this->get_mounts();
		this->mark_mounted();
	}

	list<Hash> GetDisks();
	list<Device> GetPhysicalDisks(){return disks;}
	bool FormatPartition(const string& dev, const string& fstype);
	bool ExtendPartition(const string& dev);
	static bool CreateSimplePart(const string& dev, enum PartType parttype, const string& label );
	static bool SetPartitionType(const string& dev, const string& partition, enum PartType parttype);
	static bool Probe( const string& dev );
	static bool IsMounted(const string& dev);

	virtual ~Disks(){};
private:
	list<Device> disks;
	list<Hash> mounts;

	static string get_bus(const string& device);
	static string get_partitiontype(const string& dev, int partno);
	void get_mounts();
	void get_disks();
	void mark_mounted();
	void get_partitions(Device& dev,const string& disk);

	friend class LVM;
	friend class RaidDevs;

};


#endif

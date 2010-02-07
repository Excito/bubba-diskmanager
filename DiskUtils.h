
#ifndef DISKUTILS_H
#define DISKUTILS_H

#include <map>
#include <string>
#include <list>

#include "Utils.h"

typedef struct{
	Hash attr;
	std::list<Hash> slaves;
	std::list<Hash> logics;
	hashlist logics_devices;
} Device;

class DiskMetaData{
public:
	static DiskMetaData& Instance();
	std::string GetUUID(const std::string& dev);
	std::string GetDev(const std::string& uuid);
	std::string GetLabel(const std::string& dev);
	void Refresh();
private:
	DiskMetaData();
	DiskMetaData(const DiskMetaData&){};
	Hash uuids;
	Hash ruuids;
	Hash labels;
	void get_uuid();
	void get_labels();
};

#endif

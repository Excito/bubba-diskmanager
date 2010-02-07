#ifndef LVM_H
#define LVM_H

#include "DiskUtils.h"
#include "Disks.h"
#include <list>
#include <stdexcept>

class LVM: public StatusNotifier{
public:
	class empty_path: public std::logic_error {
	public:
		explicit empty_path(const std::string &s): std::logic_error(s) {}
		explicit empty_path(): std::logic_error("Empty path found") {}
	};
	class volumegroup_not_found: public std::runtime_error {
	public:
		explicit volumegroup_not_found(const std::string &s): std::runtime_error(s) {}
		explicit volumegroup_not_found(): std::runtime_error("Volume group not found") {}
	};
	class empty_size: public std::runtime_error {
	public:
		explicit empty_size(const std::string &s): std::runtime_error(s) {}
		explicit empty_size(): std::runtime_error("Size is empty") {}
	};
	LVM(){
		this->get_vgs();
	};
	std::list<Device> GetDevices();
	bool CreatePV(const std::string& dev);
	bool RemovePV(const std::string& dev);
	bool CreateVG(const std::string& vgname, std::list<std::string> devices);
	bool RemoveVG(const std::string& vgname);
	bool ExtendVG(const std::string& vgname, const std::string& device);
	bool CreateLV(const std::string& lvname, const std::string& vgname);
	bool RemoveLV(const std::string& lvpath);
	bool ExtendLV(const std::string& lvpath);
	void Mark(Disks& disks);
	virtual ~LVM(){};
private:
	std::list<Device> vgs;

	void get_pvs(Device& vg);
	void get_lvs(Device& vg);
	void get_vgs(void);
};

#endif

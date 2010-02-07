
#ifndef MY_FSTABCMD_H
#define MY_FSTABCMD_H

#include "CmdApp.h"

class FsTabCmd: public Cmd{

	bool do_list(Args& arg);
	bool do_add(Args& arg);
	bool do_add_by_uuid(Args& arg);
	bool do_remove(Args& arg);
	bool do_check_is_mounted(Args& arg);
	bool do_mount(Args& arg);
	bool do_umount(Args& arg);


public:
	FsTabCmd();

	bool operator()(Args& arg);

	virtual ~FsTabCmd();
};
#endif

/*
 * CmdLV.h
 *
 *  Created on: Aug 14, 2009
 *      Author: tor
 */

#ifndef CMDLV_H_
#define CMDLV_H_

#include "CmdApp.h"

class CmdLV: public Cmd{
protected:
	bool do_list(Args& arg);
	bool do_pvcreate(Args& arg);
	bool do_pvremove(Args& arg);
	bool do_vgcreate(Args& arg);
	bool do_vgremove(Args& arg);
	bool do_vgextend(Args& arg);
	bool do_lvcreate(Args& arg);
	bool do_lvremove(Args& arg);
	bool do_lvextend(Args& arg);

public:
	CmdLV();

	bool operator()(Args& arg);
};


#endif /* CMDLV_H_ */

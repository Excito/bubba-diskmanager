/*
 * CmdMD.h
 *
 *  Created on: Aug 14, 2009
 *      Author: tor
 */

#ifndef CMDMD_H_
#define CMDMD_H_

#include "CmdApp.h"

class CmdMD: public Cmd{
protected:
	bool do_list(Args& arg);
	bool do_create(Args& arg);
	bool do_get_next_md();
	bool do_assemble(Args& arg);
	bool do_stop(Args& arg);
	bool do_destroy(Args& arg);
	bool do_fail(Args& arg);
	bool do_remove(Args& arg);
	bool do_add(Args& arg);


public:
	CmdMD();

	bool operator()(Args& arg);

};


#endif /* CMDMD_H_ */

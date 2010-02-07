/*
 * DiskCmd.h
 *
 *  Created on: Aug 14, 2009
 *      Author: tor
 */

#ifndef DISKCMD_H_
#define DISKCMD_H_

#include "CmdApp.h"
#include "StatusNotifier.h"

#include <libeutils/json/json.h>

class DiskCmd: public Cmd, public StatusNotifier{
	Json::FastWriter writer;
protected:
	bool do_list(Args& arg);
	bool do_extend(Args& arg);
	bool do_format(Args& arg);
	bool do_partition(Args& arg);
	bool do_set_partition_type(Args& arg);
	bool do_probe(Args& arg);

	void update_format_status(const string& status);
	void update_extend_status(const string& status);
public:
	DiskCmd();

	bool operator()(Args& arg);

};


#endif /* DISKCMD_H_ */

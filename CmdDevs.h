/*
 * CmdDevs.h
 *
 *  Created on: Aug 14, 2009
 *      Author: tor
 */

#ifndef CMDDEVS_H_
#define CMDDEVS_H_

#include "CmdApp.h"

#include <string>

using namespace std;

class CmdDevs: public Cmd{
public:
	CmdDevs(const string& desc);
	bool operator()(Args& arg);
};

#endif /* CMDDEVS_H_ */

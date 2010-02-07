/*
 * UserMount.h
 *
 *  Created on: Aug 14, 2009
 *      Author: tor
 */

#ifndef USERMOUNT_H_
#define USERMOUNT_H_

#include <string>

#include "CmdApp.h"

using namespace std;

class UserMount: public Cmd{
public:
	UserMount(const string& desc);

	bool operator()(Args& arg);
};


#endif /* USERMOUNT_H_ */

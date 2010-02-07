/*
 * UserUmount.h
 *
 *  Created on: Aug 14, 2009
 *      Author: tor
 */

#ifndef USERUMOUNT_H_
#define USERUMOUNT_H_

#include "CmdApp.h"

class UserUmount: public Cmd{
public:
	UserUmount(const string& desc);

	bool operator()(Args& arg);

};


#endif /* USERUMOUNT_H_ */

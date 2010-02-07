/*
 * StatusNotifier.h
 *
 *  Created on: Aug 14, 2009
 *      Author: tor
 */

#ifndef STATUSNOTIFIER_H_
#define STATUSNOTIFIER_H_

#include <sigc++/sigc++.h>

#include <string>

class StatusNotifier {
	std::string status;
public:
	sigc::signal1<void, const std::string&> StatusChanged;

	StatusNotifier();

	void UpdateStatus(const std::string& status);

	std::string GetStatus(){ return status;}

	virtual ~StatusNotifier();
};

#endif /* STATUSNOTIFIER_H_ */

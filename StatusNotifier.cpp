/*
 * StatusNotifier.cpp
 *
 *  Created on: Aug 14, 2009
 *      Author: tor
 */

#include "StatusNotifier.h"

StatusNotifier::StatusNotifier() {
	// Nothing for now.

}
void StatusNotifier::UpdateStatus(const std::string& status){
	this->status=status;
	this->StatusChanged.emit(this->status);
}


StatusNotifier::~StatusNotifier() {
	// Nothing for now.
}

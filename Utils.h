/*
 * Utils.h
 *
 *  Created on: Aug 14, 2009
 *      Author: tor
 */

#ifndef UTILS_H_
#define UTILS_H_

#include <map>
#include <list>
#include <string>

#include <libeutils/json/json.h>

typedef std::map<std::string,std::string> Hash;
typedef std::list<std::string> stringlist;
typedef std::map<std::string,stringlist> hashlist;



Json::Value fromHash(Hash& h);

#endif /* UTILS_H_ */

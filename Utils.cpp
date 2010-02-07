/*
 * Utils.cpp
 *
 *  Created on: Aug 14, 2009
 *      Author: tor
 */

#include "Utils.h"

Json::Value fromHash(Hash& h){
	Json::Value ret(Json::objectValue);
	for(Hash::iterator hIt=h.begin();hIt!=h.end();hIt++){
		ret[(*hIt).first]=(*hIt).second;
	}
	return ret;
}

#if 0

#include <algorithm>
#include <functional>


template <class T> void wl(T val){
	cout << "["<<val<<"]" << endl;
}

void wm(pair<string,string> val){
	cout <<"Key: ["<<val.first<<"]\tvalue: ["<<val.second<<"]"<<endl;
}


class Dumper{
	string label;
	string slave;
public:
	Dumper(){};

	void SetLabel(const string& l,const string& s=""){
		label=l;
		slave=s;
	}

	void operator()(Device& dev){
		cout << endl<<"========  "<<label<<"  ======="<<endl;
		for_each(dev.attr.begin(),dev.attr.end(),wm);
		for(list<Hash>::iterator hIt=dev.slaves.begin(); hIt!=dev.slaves.end();hIt++){
			cout << "\t ----------"<<slave<<"-------------"<<endl;
			for(Hash::iterator slaveIt=(*hIt).begin();slaveIt!=(*hIt).end();slaveIt++){
				cout <<"\t key: ["<<(*slaveIt).first<< "]\tvalue: ["<< (*slaveIt).second<<"]"<<endl;
			}
		}
	}

	void operator()(Hash& h){
		cout << "--------"<< label << "-----------"<<endl;
		for(Hash::iterator hIt=h.begin();hIt!=h.end();hIt++){
			cout <<"\t key: ["<<(*hIt).first<< "]\tvalue: ["<< (*hIt).second<<"]"<<endl;
		}
	}
};

#endif

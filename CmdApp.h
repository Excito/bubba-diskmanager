#ifndef CMDAPP_H
#define CMDAPP_H
#include <list>
#include <vector>
#include <string>
#include <map>

using namespace std;

typedef vector<string> Args;


class Cmd{
public:
	string description;
	Cmd(const string& desc=""):description(desc){}
	string getDescription(){return this->description;}
	virtual bool operator()(Args& arg)=0;
	virtual ~Cmd(){}
};


class CmdApp{
public:
	typedef map<string,Cmd*> CmdMap;

	CmdApp(string name=""):appname(name){}
	CmdApp(CmdMap cmds):cmap(cmds){}

	void AddCmd(string name, Cmd* cmd){ this->cmap[name]=cmd;}

	bool Help();
	bool Run(int argc, char** argv);

	virtual ~CmdApp();

private:
	string appname;
	CmdMap cmap;
};


#endif

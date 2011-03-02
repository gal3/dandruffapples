#ifndef _HELPERFN_H_
#define _HELPERFN_H_

#include <iostream>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <map>

using namespace std;

//comment to disable generation of debug for certain programs
//#define DEBUG
//#define ENABLE_LOGGING

namespace helper {

string toString(int);
string getNewName(string);

class CmdLine {
public:
	CmdLine(int, char**);

	//get the value of an argument from the command line
	string getArg(string arg, string defaultVal="", int maxLength=-1);
private:
	map<string, string> parsedCmdLine;

};

const string defaultLogName = "antix_log";
const string worldViewerDebugLogName="/tmp/worldviewer_debug.txt";
const string clientViewerDebugLogName="/tmp/clientviewer_debug.txt";
}

#endif

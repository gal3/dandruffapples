#ifndef _HELPERFN_H_
#define _HELPERFN_H_

#include <iostream>
#include <sstream>
#include <string>
#include <math.h>
#include <sys/stat.h>
#include <map>

using namespace std;

namespace helper {
template <class T>
inline string toString (const T& t){
	stringstream ss;
	ss << t;

	return ss.str();;
}
float distanceBetween(float x1, float x2, float y1, float y2);

class CmdLine {
public:
	CmdLine(int, char**);

	//get the value of an argument from the command line
	string getArg(string arg, string defaultVal="", int maxLength=-1);
private:
	map<string, string> parsedCmdLine;

};

const string scoreKeeperFinalScore = "finalscore.txt";

const string logDirectory="/tmp/";
const string scoreKeeperLog = "scorekeeperlog";
const string worldViewerDebugLogName="worldviewer_debug.txt";
const string clientViewerDebugLogName="clientviewer_debug.txt";
}

struct TwoInt{
  int one, two;

  TwoInt(int first, int second) : one(first), two(second) {}
};

const unsigned int bytePackClear = 65535;

unsigned int BytePack(int a, int b);
TwoInt ByteUnpack(unsigned int data);

float hack_angleFromVelocity(double x, double y);

#endif

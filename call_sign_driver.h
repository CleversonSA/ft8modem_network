#include <iostream>
#include <map>
#include <string>

using std::map;
using std::pair;
using std::string;
using std::cout;
using std::endl;

#ifndef CALLSIGNCOUNTRYDRIVER
#define CALLSIGNCOUNTRYDRIVER

class CallSignCountryDriver {
private:
    map<pair<string, string>, string> callSigns;

public:
    CallSignCountryDriver();
    string getCountry(string &callSign);
};

#endif

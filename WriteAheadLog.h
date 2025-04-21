#ifndef WRITEAHEADLOG_H
#define WRITEAHEADLOG_H
#include <fstream>
#include <string>
#include <vector>
#include <utility> // for pair

using namespace std;

class WriteAheadLog {
private:
    ofstream logFile;

public:
    WriteAheadLog();  
    void logWrite(const string& key, const string& value);
    vector<pair<string, string>> loadLog();  // for recovery
};

#endif
#ifndef WRITEAHEADLOG_H
#define WRITEAHEADLOG_H
#include <fstream>
#include <string>
#include <vector>
#include <utility> 

using namespace std;

class WriteAheadLog {
private:
    ofstream logFile;

public:
    WriteAheadLog();  
    void logWrite(const string& key, const vector<string>& attrs);
    vector<pair<string, vector<string>>> loadLog();
};

#endif
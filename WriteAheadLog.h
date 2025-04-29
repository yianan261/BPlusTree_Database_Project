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
    void logWrite(const string& tableName, const string& key, const vector<string>& attrs, const vector<string>& headers);
    vector<tuple<string, string, vector<pair<string,string>>>> loadLog();

};

#endif
#include "WriteAheadLog.h"
#include <fstream>
#include <sstream>
#include<iostream>

using namespace std;

WriteAheadLog::WriteAheadLog(){
    logFile.open("wal.log", ios::app); //append
    if (!logFile.is_open()){
        cerr << "Failed to open wal.log for writing. \n";
    }
}

void WriteAheadLog::logWrite(const string& tableName, const string& key, const vector<string>& attrs, const vector<string>& headers){
        if(!logFile.is_open()) return;
        logFile << tableName << "," << key;
        for (size_t i = 0; i < attrs.size(); ++i) {
            logFile << (i == 0 ? "," : "|") << headers[i] << "=" << attrs[i];
        }
        logFile << "\n";
}

vector<tuple<string, string, vector<pair<string,string>>>> WriteAheadLog::loadLog()
 {
    vector<tuple<string, string, vector<pair<string,string>>>> logEntries;
    ifstream infile("wal.log");
    if (!infile.is_open()) {
        cerr << "Failed to open wal.log\n";
        return logEntries;
    }

    string line;
    while (getline(infile, line)) {
        stringstream ss(line);
        string tableName, key, rest;
        if (getline(ss, tableName, ',') && getline(ss, key, ',') && getline(ss, rest)) {
            vector<pair<string,string>> attrs;
            string attr;
            stringstream ss2(rest);
            while (getline(ss2, attr, '|')) {
                auto pos = attr.find('=');
                if (pos != string::npos) {
                    string header = attr.substr(0, pos);
                    string value = attr.substr(pos + 1);
                    attrs.emplace_back(header, value);
                }
            }
            logEntries.emplace_back(tableName, key, attrs);
        }
    }
    return logEntries;

}

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

void WriteAheadLog::logWrite(const string& key, const vector<string>& attrs){
        if(!logFile.is_open()) return;
        logFile << key;
        for (size_t i = 0; i < attrs.size(); ++i) {
            logFile << (i==0 ? "," : "|") << attrs[i];  // ,key|attr1|attr2...
        }
       logFile << "\n";
}

vector<pair<string, vector<string>>> WriteAheadLog::loadLog() {
    vector<pair<string, vector<string>>> logEntries;
    ifstream infile("wal.log");

    if(!infile.is_open()){
        cerr << "Failed to open wal.log for reading. \n";
        return logEntries;
    }

    string line;
    while(getline(infile,line)){
        stringstream ss(line);
        string key, rest;
        if(getline(ss, key, ',') && getline(ss, rest)){
            vector<string> attrs;
            string attr;
            stringstream ss2(rest);
            while(getline(ss2, attr, '|')) attrs.push_back(attr);
            logEntries.emplace_back(key, attrs);
        }
    }
    return logEntries;
}

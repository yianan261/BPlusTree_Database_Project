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

void WriteAheadLog::logWrite(const string& key, const string& value){
    if(logFile.is_open()){
        logFile << key << "," << value << "\n";
        logFile.flush(); 
    }
}

vector<pair<string, string>> WriteAheadLog::loadLog(){
    vector<pair<string, string>> logEntries;
    ifstream infile("wal.log");

    if(!infile.is_open()){
        cerr << "Failed to open wal.log for reading. \n";
        return logEntries;
    }

    string line;
    while(getline(infile,line)){
        stringstream ss(line);
        string key, value;
        if(getline(ss,key,',') && getline(ss,value)){
            logEntries.emplace_back(key,value);
        }
    }

    return logEntries;
}

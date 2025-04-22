#ifndef LEADERDB_H
#define LEADERDB_H

#include "DBInstance.h"
#include "StorageEngine.h"
#include "BTreeIndex.h"
#include "WriteAheadLog.h"

using namespace std;

//inherits publicly from DBInstance
class LeaderDB : public DBInstance {
private:
    StorageEngine storage;
    WriteAheadLog wal;
    BTreeIndex index;  

public:
    void set(const string& key, const vector<string> & attrs) override;
    void recoverFromWAL();
    vector<string> get(const string& key) override;
    void deleteKey(const string& key) override;
    vector<vector<string>> getPrefix(const string& prefixKey) override;
};

#endif
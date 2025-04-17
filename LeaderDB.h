#ifndef LEADERDB_H
#define LEADERDB_H

#include "DBInstance.h"
#include "StorageEngine.h"
#include "BTreeIndex.h"

using namespace std;

//inherits publicly from DBInstance
class LeaderDB : public DBInstance {
private:
    StorageEngine storage;
    BTreeIndex index;

public:
    void set(const string& key, const string& value) override;
    string get(const string& key) override;
    void deleteKey(const string& key) override;
    vector<string> getPrefix(const string& prefixKey) override;
};

#endif
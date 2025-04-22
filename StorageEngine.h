#ifndef STORAGEENGINE_H
#define STORAGEENGINE_H

#include "BTreeIndex.h"
#include <string>
#include <vector>
#include <map>

using namespace std;

class StorageEngine {
private:
    map<string, BTreeIndex> tables;  
    string currentTable;             

public:
    StorageEngine() : currentTable("default") {
        tables["default"] = BTreeIndex();
    }

    void createTable(const string& tableName);
    void dropTable(const string& tableName);
    void switchTable(const string& tableName);
    string getCurrentTable() const;
    bool hasTable(const string& tableName) const;
    vector<string> listTables() const;

    void set(const string& key, const vector<string>& attrs);
    vector<string> get(const string& key);
    vector<vector<string>> getPrefix(const string& prefix);
    void deleteKey(const string& key);
    void clear();
};

#endif
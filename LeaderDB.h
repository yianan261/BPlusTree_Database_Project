#ifndef LEADERDB_H
#define LEADERDB_H

#include "DBInstance.h"
#include "BTreeIndex.h"
#include "WriteAheadLog.h"
#include <map>
#include <sstream>

using namespace std;

class LeaderDB : public DBInstance {
private:
    WriteAheadLog wal;
    map<string, BTreeIndex> tables;  
    string currentTable;             

public:
    LeaderDB() : currentTable("default") {
        tables["default"] = BTreeIndex();
    }

    // curd operations
    void create(const string& key, const vector<string>& attrs) override;
    void update(const string& key, const vector<string>& attrs) override;
    vector<string> get(const string& key) override;
    void deleteKey(const string& key) override;
    vector<vector<string>> getPrefix(const string& prefixKey) override;

    // range query
    vector<vector<string>> getRange(const string& lowKey, const string& highKey);
    
    // multiple table management
    void createTable(const string& tableName);
    void dropTable(const string& tableName);
    void switchTable(const string& tableName);
    bool hasTable(const string& tableName) const;
    vector<string> listTables() const;
    string getCurrentTable() const;
    void recoverFromWAL();

    // file import functionality
    bool loadFromFile(const string& tableName, const string& filepath);
};

#endif
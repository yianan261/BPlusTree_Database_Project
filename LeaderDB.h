#ifndef LEADERDB_H
#define LEADERDB_H

#include "DBInstance.h"
#include "BTreeIndex.h"
#include "WriteAheadLog.h"
#include <map>

using namespace std;

class LeaderDB : public DBInstance {
private:
    WriteAheadLog wal;
    map<string, BTreeIndex> tables;  // 表名到B树索引的映射
    string currentTable;             // 当前活动表名

public:
    LeaderDB() : currentTable("default") {
        tables["default"] = BTreeIndex();
    }

    // curd operations
    void set(const string& key, const vector<string>& attrs) override;
    vector<string> get(const string& key) override;
    void deleteKey(const string& key) override;
    vector<vector<string>> getPrefix(const string& prefixKey) override;
    
    // multiple table management
    void createTable(const string& tableName);
    void dropTable(const string& tableName);
    void switchTable(const string& tableName);
    bool hasTable(const string& tableName) const;
    vector<string> listTables() const;
    string getCurrentTable() const;
    void recoverFromWAL();
};

#endif
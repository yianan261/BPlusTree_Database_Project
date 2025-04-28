#ifndef LEADERDB_H
#define LEADERDB_H

#include "DBInstance.h"
#include "BTreeIndex.h"
#include "WriteAheadLog.h"
#include "SecondaryIndex.h"
#include "BPlusTree.h"
#include <map>
#include <unordered_map>
#include <sstream>

using namespace std;

class LeaderDB : public DBInstance {
private:
    WriteAheadLog wal;
    map<string, BTreeIndex> tables;  
    string currentTable;  
    unordered_map<string, unordered_map<int, SecondaryIndex>> secondary;           

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

    // get btree index
    BTreeIndex& getCurrentIndex();
    
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

    // secondary index management WHERE
    void createSecondaryIdx(int attrIdx);
    vector<vector<string>> findByAttr(int attrIdx, const string& value);
    vector<vector<string>> selectWhere(const vector<int>& projCols, int whereCol, const string& whereVal);

    // join
    vector<vector<string>> join(const string& tabA, int colA, const string& tabB, int colB, 
        const vector<int>& projA, const vector<int>& projB);

    // headers management
    void setTableHeaders(const string& tableName, const vector<string>& headers);
    vector<string> getTableHeaders(const string& tableName) const;
    int getColumnIndex(const string& tableName, const string& columnName) const;

    // export table to csv
    bool exportTableToCsv(const string& tableName, const string& dirPath);
};

#endif
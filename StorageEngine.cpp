#include "StorageEngine.h"
#include <climits>         

void StorageEngine::createTable(const string& tableName) {
    if (tables.find(tableName) == tables.end()) {
        tables[tableName] = BTreeIndex();
    }
}

void StorageEngine::dropTable(const string& tableName) {
    if (tableName != "default") {
        tables.erase(tableName);
        if (currentTable == tableName) {
            currentTable = "default";
        }
    }
}

void StorageEngine::switchTable(const string& tableName) {
    if (tables.find(tableName) != tables.end()) {
        currentTable = tableName;
    }
}

string StorageEngine::getCurrentTable() const {
    return currentTable;
}

bool StorageEngine::hasTable(const string& tableName) const {
    return tables.find(tableName) != tables.end();
}

vector<string> StorageEngine::listTables() const {
    vector<string> tableNames;
    for (const auto& pair : tables) {
        tableNames.push_back(pair.first);
    }
    return tableNames;
}

void StorageEngine::set(const string& key, const vector<string>& attrs) {
    tables[currentTable].insert(key, attrs);
}

vector<string> StorageEngine::get(const string& key) {
    return tables[currentTable].search(key);
}

void StorageEngine::deleteKey(const string& key) {
    tables[currentTable].remove(key);
}

vector<vector<string>> StorageEngine::getPrefix(const string& prefix) {
    string high = prefix; high.push_back(CHAR_MAX);
    vector<vector<string>> result;
    for (const auto& item : tables[currentTable].rangeQuery(prefix, high)) {
        result.push_back({item});
    }
    return result;
}

void StorageEngine::clear(){
    tables[currentTable].clear();
}

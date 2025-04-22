#include "LeaderDB.h"

void LeaderDB::set(const string& key, const vector<string>& attrs) {
    wal.logWrite(key, attrs);
    tables[currentTable].insert(key, attrs);
}

vector<string> LeaderDB::get(const string& key) {
    return tables[currentTable].search(key);
}

void LeaderDB::deleteKey(const string& key) {
    tables[currentTable].remove(key);
}

vector<vector<string>> LeaderDB::getPrefix(const string& prefixKey) {
    string high = prefixKey; high.push_back(CHAR_MAX);
    vector<vector<string>> result;
    for (const auto& item : tables[currentTable].rangeQuery(prefixKey, high)) {
        result.push_back({item});
    }
    return result;
}


void LeaderDB::createTable(const string& tableName) {
    if (tables.find(tableName) == tables.end()) {
        tables[tableName] = BTreeIndex();
    }
}

void LeaderDB::dropTable(const string& tableName) {
    if (tableName != "default") {
        tables.erase(tableName);
        if (currentTable == tableName) {
            currentTable = "default";
        }
    }
}

void LeaderDB::switchTable(const string& tableName) {
    if (tables.find(tableName) != tables.end()) {
        currentTable = tableName;
    }
}

bool LeaderDB::hasTable(const string& tableName) const {
    return tables.find(tableName) != tables.end();
}

vector<string> LeaderDB::listTables() const {
    vector<string> tableNames;
    for (const auto& pair : tables) {
        tableNames.push_back(pair.first);
    }
    return tableNames;
}

string LeaderDB::getCurrentTable() const {
    return currentTable;
}

void LeaderDB::recoverFromWAL() {
    tables.clear();
    tables["default"] = BTreeIndex();
    currentTable = "default";
    
    auto entries = wal.loadLog();
    for (const auto& [key, val] : entries) {
        tables[currentTable].insert(key, {val});
    }
}

bool LeaderDB::loadFromFile(const string& tableName, const string& filepath) {
    if (tables.find(tableName) == tables.end()) {
        return false;
    }

    ifstream file(filepath);
    if (!file.is_open()) {
        return false;
    }

    string line;
    while (getline(file, line)) {
        stringstream ss(line);
        string field, key;
        vector<string> attrs;
        
        if (getline(ss, key, ',')) {
            while (getline(ss, field, ',')) {
                attrs.push_back(field);
            }
            tables[tableName].insert(key, attrs);
        }
    }
    file.close();
    return true;
}

#include "LeaderDB.h"

vector<string> LeaderDB::get(const string& key) {
    return tables[currentTable].search(key);
}

void LeaderDB::deleteKey(const string& key) {
    auto oldAttrs = tables[currentTable].search(key);
    int pk = stoi(key);
    tables[currentTable].remove(key);
    for (auto& [c, idx] : secondary[currentTable]) 
        idx.remove(oldAttrs, pk);
}

void LeaderDB::create(const string& key, const vector<string>& attrs) {
    int keyInt = stoi(key);
    if (tables[currentTable].contains(keyInt)) {
        throw runtime_error("Key already exists. Use update instead.");
    }
    wal.logWrite(key, attrs);
    tables[currentTable].insert(key, attrs);
    for (auto& [c, idx] : secondary[currentTable]) 
        idx.insert(attrs, keyInt);
}

void LeaderDB::update(const string& key, const vector<string>& attrs) {

    int pk = stoi(key);
    if (!tables[currentTable].contains(pk))
        throw runtime_error("Key does not exist. Use create instead.");
    wal.logWrite(key, attrs);

    auto oldAttrs = tables[currentTable].search(key);
    for (auto& [c, idx] : secondary[currentTable])
        idx.remove(oldAttrs, pk);

    tables[currentTable].update(key, attrs);

    for (auto& [c, idx] : secondary[currentTable])
        idx.insert(attrs, pk);
}

vector<vector<string>> LeaderDB::getPrefix(const string& prefixKey) {
    string high = prefixKey; high.push_back(CHAR_MAX);
    vector<vector<string>> result;
    for (const auto& item : tables[currentTable].rangeQuery(prefixKey, high)) {
        result.push_back({item});
    }
    return result;
}

vector<vector<string>> LeaderDB::getRange(const string& lowKey, const string& highKey){
    return tables[currentTable].rangeQuery(lowKey, highKey);
}

BTreeIndex& LeaderDB::getCurrentIndex(){
    return tables[currentTable];
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
void LeaderDB::createSecondaryIdx(int col)
{
    auto& secMap = secondary[currentTable];
    if (secMap.count(col)) return;
    secMap.emplace(col, SecondaryIndex(col));

    auto& prim = tables[currentTable].raw();
    vector<pair<int,vector<string>>> rows;
    prim.forEachLeaf([&](const Entry<int, vector<string>>& e){  
        rows.emplace_back(e.key, e.attrs);
    });
    secMap[col].bulkBuild(rows.begin(), rows.end());
}

/* ---------- findByAttr ---------- */
vector<vector<string>> LeaderDB::findByAttr(int col, const string& val)
{
    auto& secMap = secondary[currentTable];
    if (!secMap.count(col)) return {};
    auto pks = secMap[col].searchPK(val);

    vector<vector<string>> res;
    for (int pk : pks)
        res.push_back(tables[currentTable].search(to_string(pk)));
    return res;
}


vector<vector<string>> LeaderDB::selectWhere(const vector<int>& projCols, int whereCol, const string& whereVal)
{
    vector<vector<string>> rows;
    
    auto tIt = secondary.find(currentTable);
    if (tIt != secondary.end() && tIt->second.count(whereCol)) {
        rows = findByAttr(whereCol, whereVal);
    } else {
        
        auto& prim = tables[currentTable].raw();
        prim.forEachLeaf([&](const Entry<int,vector<string>>& e){
            if (whereCol < (int)e.attrs.size() &&
                e.attrs[whereCol] == whereVal)
                rows.push_back(e.attrs);
        });
    }

    
    if (projCols.empty()) return rows;              
    vector<vector<string>> proj;
    proj.reserve(rows.size());
    for (auto& r : rows) {
        vector<string> tmp;
        for (int c : projCols)
            if (c < (int)r.size()) tmp.push_back(r[c]);
        proj.push_back(std::move(tmp));
    }
    return proj;
}

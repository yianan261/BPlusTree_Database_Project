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
            if (c < (int)r.size()) 
                tmp.push_back(r[c]);
        proj.push_back(std::move(tmp));
    }
    return proj;
}

vector<vector<string>> LeaderDB::join(const string& tabA, int colA, const string& tabB, int colB,
    const vector<int>& projA, const vector<int>& projB)
{

    if (!tables.count(tabA) || !tables.count(tabB))
        throw runtime_error("Table not found");

    auto& tA = tables[tabA];
    auto& tB = tables[tabB];

    
    auto outerName = tabA, innerName = tabB;
    int  oCol = colA, iCol = colB;
    auto *outer = &tA, *inner = &tB;

    if (tB.size() < tA.size()){
        outerName.swap(innerName);
        std::swap(oCol,iCol);
        std::swap(outer,inner);
    }

    /* 2. 预取 inner 的可能索引 */
    bool innerHasIdx =
        secondary[innerName].count(iCol);

    vector<vector<string>> result;

    /* 3. 遍历 outer */
    outer->raw().forEachLeaf([&](const Entry<int,vector<string>>& eO){
        if (oCol >= (int)eO.attrs.size()) return;
        const string& matchVal = eO.attrs[oCol];

        /* 3.1 获取 inner 匹配行集合 */
        vector<vector<string>> rowsI;
        if (innerHasIdx)
            rowsI = findByAttr(iCol, matchVal);               // uses index
        else {
            inner->raw().forEachLeaf([&](const auto& eI){
                if (iCol < (int)eI.attrs.size() &&
                    eI.attrs[iCol]==matchVal)
                    rowsI.push_back(eI.attrs);
            });
        }

        /* 3.2 生成拼接输出 */
        for (auto& rI : rowsI){
            vector<string> row;
            // 投影 A
            if (projA.empty()) row.insert(row.end(), eO.attrs.begin(), eO.attrs.end());
            else
                for (int c:projA)
                    if (c < (int)eO.attrs.size()) 
                        row.push_back(eO.attrs[c]);
            // 投影 B
            if (projB.empty()) 
                row.insert(row.end(), rI.begin(), rI.end());
            else
                for (int c:projB)
                    if (c < (int)rI.size()) 
                        row.push_back(rI[c]);

            result.push_back(std::move(row));
        }
    });
    return result;
}
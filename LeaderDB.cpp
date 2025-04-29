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
    wal.logWrite(currentTable, key, attrs, getTableHeaders(currentTable));
    tables[currentTable].insert(key, attrs);
    for (auto& [c, idx] : secondary[currentTable]) 
        idx.insert(attrs, keyInt);
}

void LeaderDB::update(const string& key, const vector<string>& attrs) {
    auto oldAttrs = tables[currentTable].search(key);
    if (oldAttrs.empty())
        throw runtime_error("Key does not exist. Use create instead.");

    wal.logWrite(currentTable, key, attrs, getTableHeaders(currentTable));

    int pk = stoi(key);

    for (auto& [c, idx] : secondary[currentTable])
        idx.remove(oldAttrs, pk);

    tables[currentTable].update(key, attrs);

    for (auto& [c, idx] : secondary[currentTable])
        idx.insert(attrs, pk);
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
    currentTable = "default";
    tables["default"] = BTreeIndex();
    
    auto entries = wal.loadLog();
    for(const auto& [tableName, key, attrPairs] : entries){
        if (!tables.count(tableName)){
            tables[tableName] = BTreeIndex(); // if missing, create
        }
        vector<string> attrs;
        for (const auto& [header, value]: attrPairs){
            attrs.push_back(value);
        }

        tables[tableName].insert(key,attrs);
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

    
    bool innerHasIdx =
        secondary[innerName].count(iCol);

    vector<vector<string>> result;

    
    outer->raw().forEachLeaf([&](const Entry<int,vector<string>>& eO){
        if (oCol >= (int)eO.attrs.size()) return;
        const string& matchVal = eO.attrs[oCol];

        
        vector<vector<string>> rowsI;
        if (innerHasIdx)
            rowsI = findByAttr(iCol, matchVal);              
        else {
            inner->raw().forEachLeaf([&](const auto& eI){
                if (iCol < (int)eI.attrs.size() &&
                    eI.attrs[iCol]==matchVal)
                    rowsI.push_back(eI.attrs);
            });
        }

        
        for (auto& rI : rowsI){
            vector<string> row;
            
            if (projA.empty()) row.insert(row.end(), eO.attrs.begin(), eO.attrs.end());
            else
                for (int c:projA)
                    if (c < (int)eO.attrs.size()) 
                        row.push_back(eO.attrs[c]);
            
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

void LeaderDB::setTableHeaders(const string& tableName, const vector<string>& headers) {
    if (tables.find(tableName) != tables.end()) {
        tables[tableName].setHeaders(headers);
    }
}

vector<string> LeaderDB::getTableHeaders(const string& tableName) const {
    auto it = tables.find(tableName);
    if (it != tables.end()) {
        return it->second.getHeaders();
    }
    return vector<string>();
}

int LeaderDB::getColumnIndex(const string& tableName, const string& columnName) const {
    auto it = tables.find(tableName);
    if (it != tables.end()) {
        return it->second.getColumnIndex(columnName);
    }
    return -1;
}


string escapeCSV(const string& str) {
    bool needQuotes = str.find(',') != string::npos;
    if (!needQuotes) return str;
    
    string escaped = "\"";
    for (char c : str) {
        if (c == '\"') escaped += "\"\"";  
        else escaped += c;
    }
    escaped += "\"";
    return escaped;
}

bool LeaderDB::exportTableToCsv(const string& tableName, const string& dirPath) {
    if (!hasTable(tableName)) return false;
    
    string filepath = dirPath + "/" + tableName + ".csv";
    ofstream file(filepath);
    if (!file.is_open()) return false;

    
    auto headers = getTableHeaders(tableName);
    for (size_t i = 0; i < headers.size(); i++) {
        if (i > 0) file << ",";
        file << escapeCSV(headers[i]);
    }
    file << "\n";

    
    auto& btree = tables[tableName].raw();
    btree.forEachLeaf([&](const Entry<int, vector<string>>& e) {
        file << e.key;  
        for (const auto& attr : e.attrs) {
            file << "," << escapeCSV(attr);
        }
        file << "\n";
    });

    file.close();
    return true;
}
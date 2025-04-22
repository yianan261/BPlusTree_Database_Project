#include "LeaderDB.h"

void LeaderDB::set(const string& key, const vector<string>& attrs) {
    wal.logWrite(key, attrs); 
    storage.set(key, attrs); 
    index.insert(key, attrs);
}

void LeaderDB::recoverFromWAL() {
    storage.clear();
    auto entries = wal.loadLog();
    for (const auto& [key, val]: entries) {
        storage.set(key, val);
        vector<string> attrs = {val};  // 将 string 转换为 vector<string>
        index.insert(key, attrs);
    }
}

vector<string> LeaderDB::get(const string& key) {
    auto results = index.search(key);
    if (!results.empty()) {
        return storage.get(key);
    }
    return {};
}

void LeaderDB::deleteKey(const string& key) {
    storage.deleteKey(key);
}

vector<vector<string>> LeaderDB::getPrefix(const string& prefixKey) {
    return storage.getPrefix(prefixKey);
}

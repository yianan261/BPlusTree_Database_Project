#include "StorageEngine.h"
#include <climits>         // for CHAR_MAX

void StorageEngine::set(const string& key, const vector<string>& attrs) {
    tree.insert(key, attrs);
}

vector<string> StorageEngine::get(const string& key) {
    return tree.search(key);  
}

void StorageEngine::deleteKey(const string& key) {
    tree.remove(key);
}

vector<vector<string>> StorageEngine::getPrefix(const string& prefix) {
    string high = prefix; high.push_back(CHAR_MAX);
    vector<vector<string>> result;
    for (const auto& item : tree.rangeQuery(prefix, high)) {
        result.push_back({item});
    }
    return result;
}

void StorageEngine::clear(){
    tree.clear();
}

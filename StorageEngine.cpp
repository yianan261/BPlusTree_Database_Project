#include "StorageEngine.h"
#include <algorithm>

void StorageEngine::set(const string& key, const string& value) {
    //kvStore[key] = value;
    // Using emplace to substitute the above line for better performance
    kvStore.emplace(key, value); 
}

string StorageEngine::get(const string& key) {
    auto it = kvStore.find(key);
    return (it != kvStore.end()) ? it->second : "Not Found";
}

void StorageEngine::deleteKey(const string& key) {
    kvStore.erase(key);
}

vector<string> StorageEngine::getPrefix(const string& prefix) {
    vector<string> result;
    for (const auto& pair : kvStore) {
        if (pair.first.substr(0, prefix.length()) == prefix) {
            result.push_back(pair.second);
        }
    }
    return result;
}

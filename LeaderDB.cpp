#include "LeaderDB.h"

void LeaderDB::set(const string& key, const string& value) {
    wal.logWrite(key,value); // log before writing
    storage.set(key, value);
    index.insert(key, value);
}

void LeaderDB::recoverFromWAL(){
    storage.clear();
    index.clear();
    auto entries = wal.loadLog();
    for (const auto& [key,val]: entries){
        storage.set(key,val);
        index.insert(key,val);
    }
}

string LeaderDB::get(const string& key) {
    if (index.search(key) != "") {
    return storage.get(key);
}
    return "Not Found";
}

void LeaderDB::deleteKey(const string& key) {
    storage.deleteKey(key);
    index.remove(key);
}

vector<string> LeaderDB::getPrefix(const string& prefixKey) {
    return storage.getPrefix(prefixKey);
}

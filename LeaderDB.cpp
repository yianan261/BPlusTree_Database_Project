#include "LeaderDB.h"

void LeaderDB::set(const string& key, const string& value) {
    storage.set(key, value);
    index.insert(key, value);
}

string LeaderDB::get(const string& key) {
    return storage.get(key);
}

void LeaderDB::deleteKey(const string& key) {
    storage.deleteKey(key);
    index.remove(key);
}

vector<string> LeaderDB::getPrefix(const string& prefixKey) {
    return storage.getPrefix(prefixKey);
}

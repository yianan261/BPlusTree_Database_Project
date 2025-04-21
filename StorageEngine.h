#ifndef STORAGEENGINE_H
#define STORAGEENGINE_H

#include <unordered_map>
#include <string>
#include <vector>

using namespace std;

class StorageEngine {
private:
    unordered_map<string, string> kvStore;

public:
    void set(const string& key, const string& value);
    string get(const string& key);
    void deleteKey(const string& key);
    vector<string> getPrefix(const string& prefix);
    void clear();
};

#endif
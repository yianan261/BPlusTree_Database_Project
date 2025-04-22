#ifndef STORAGEENGINE_H
#define STORAGEENGINE_H

#include "BTreeIndex.h"
#include <string>
#include <vector>

using namespace std;

class StorageEngine {
private:
    BTreeIndex tree;  

public:
    void set(const string& key, const vector<string>& attrs);
    vector<string> get(const string& key);
    vector<vector<string>> getPrefix(const string& prefix);
    void deleteKey(const string& key);
    void clear();
};

#endif
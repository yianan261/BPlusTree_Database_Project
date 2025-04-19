#ifndef BTREEINDEX_H
#define BTREEINDEX_H
#include "BPlusTree.h"
#include <string>
#include <vector>

using namespace std;

class BTreeIndex {
private:
    BPlusTree tree;

public:
    void insert(const string& key, const string& value);
    string search(const string& key);
    vector<string> rangeQuery(const string& lowKey, const string& highKey);
    void remove(const string& key);
    void print();
};

#endif
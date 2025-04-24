#ifndef BTREEINDEX_H
#define BTREEINDEX_H
#include "BPlusTree.h"
#include <string>
#include <vector>

using namespace std;

using PrimTree = BPlusTree<int, vector<string>>;
class BTreeIndex {
private:
    PrimTree tree;

public:
    void insert(const string& key, const vector<string>& attrs);
    vector<string> search(const string& key);
    vector<vector<string>> rangeQuery(const string& lowKey, const string& highKey);
    void update(const string& key, const vector<string>& attrs);
    bool contains(int key) const;
    void remove(const string& key);
    void clear();
    void print();
    PrimTree& raw() { return tree; }
};

#endif
#ifndef BTREEINDEX_H
#define BTREEINDEX_H
#include "BPlusTree.h"
#include <string>
#include <vector>

class BTreeIndex {
private:
    BPlusTree tree;

public:
    void insert(const std::string& key, const std::string& value);
    std::string search(const std::string& key);
    std::vector<std::string> rangeQuery(const std::string& lowKey, const std::string& highKey);
    void remove(const std::string& key);
    void print();
};

#endif
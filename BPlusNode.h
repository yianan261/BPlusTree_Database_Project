#pragma once
#include <vector>
#include <string>

struct Entry {
    int key;
    std::string value;
};

class BPlusNode {
public:
    bool isLeaf;
    std::vector<int> keys;
    std::vector<BPlusNode*> children;
    std::vector<Entry> entries;
    BPlusNode* next;
    BPlusNode* prev;

    BPlusNode(bool leaf);
};

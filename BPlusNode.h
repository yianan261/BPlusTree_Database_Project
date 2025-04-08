#ifndef BPLUSTREE_NODE_H
#define BPLUSTREE_NODE_H
#include <vector>
#include <string>
using namespace std;

struct Entry {
    int key;
    string value;
};

class BPlusNode {
    public:
        bool isLeaf;
        vector<int> keys;
        vector<BPlusNode*> children;              // internal node children
        vector<Entry> entries;                    // leaf node entries
        BPlusNode* next = nullptr;                // for leaf node traversal
        BPlusNode* prev = nullptr;
    
        BPlusNode(bool leaf): isLeaf(leaf) {}
    };
#endif // BPLUSTREE_NODE_H
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
    vector<BPlusNode*> children;
    vector<Entry> entries;
    BPlusNode* next;
    BPlusNode* prev;

    BPlusNode(bool leaf);
};
#endif // BPLUSTREE_NODE_H
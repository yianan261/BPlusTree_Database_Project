#ifndef BPLUSTREE_NODE_H
#define BPLUSTREE_NODE_H
#include <vector>
#include <string>
using namespace std;

struct Entry {
    int key;
    string value;
};

// base class
class BPlusNode {
    protected:
        bool isLeaf;
        vector<int> keys;
        vector<BPlusNode*> children;               // internal node children (no values only keys)
        vector<Entry> entries;                    // leaf node entries
        BPlusNode* next = nullptr;                // for leaf node traversal
        BPlusNode* prev = nullptr;

    public:
        BPlusNode(bool leaf);

        virtual ~BPlusNode() = default;
        
        virtual bool isLeafNode() const;
        vector<int>& getKeys();
        vector<BPlusNode*>& getChildren();
        virtual vector<Entry>& getEntries();
        BPlusNode* getNext() const;
        BPlusNode* getPrev() const;
        void setNext(BPlusNode* n);
        void setPrev(BPlusNode* p);

    };

class InternalNode: public BPlusNode {
    public:
        InternalNode();
        vector<Entry>& getEntries() override;
        bool isLeafNode() const override;
};

class LeafNode: public BPlusNode {
    public:
        LeafNode();
        bool isLeafNode() const override;
};

#endif // BPLUSTREE_NODE_H
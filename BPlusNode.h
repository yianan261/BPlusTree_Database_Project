#ifndef BPLUSTREE_NODE_H
#define BPLUSTREE_NODE_H
#include <vector>
#include <string>
using namespace std;

template<typename KeyT, typename PayloadT>
struct Entry {
    KeyT key;
    PayloadT attrs; 

    Entry() = default;
    Entry(const KeyT& k, const PayloadT& v) : key(k), attrs(v) {}
    bool operator<(const KeyT& rhs) const { return key < rhs; }
};

template<typename KeyT, typename PayloadT>
class BPlusNode {
    protected:
        bool isLeaf;
        vector<KeyT> keys;
        vector<BPlusNode*> children;               // internal node children (no values only keys)
        vector<Entry<KeyT, PayloadT>> entries;                    // leaf node entries
        BPlusNode* next = nullptr;                // for leaf node traversal
        BPlusNode* prev = nullptr;
        BPlusNode<KeyT, PayloadT>* parent;

    public:
        explicit BPlusNode(bool leaf);

        virtual ~BPlusNode() = default;
        
        virtual bool isLeafNode() const;
        vector<KeyT>& getKeys();
        const vector<KeyT>& getKeys() const;
        
        vector<BPlusNode*>& getChildren();
        const vector<BPlusNode*>& getChildren() const;
        
        virtual vector<Entry<KeyT, PayloadT>>& getEntries();
        virtual const vector<Entry<KeyT, PayloadT>>& getEntries() const;
        
        BPlusNode* getNext() const;
        BPlusNode* getPrev() const;
        BPlusNode<KeyT, PayloadT>* getParent() const {return parent;}
        void setNext(BPlusNode* n);
        void setPrev(BPlusNode* p);
        void setParent(BPlusNode<KeyT, PayloadT>* p){parent=p;}

};

template<typename KeyT, typename PayloadT>
class InternalNode: public BPlusNode<KeyT, PayloadT> {
    public:
        InternalNode();
        vector<Entry<KeyT, PayloadT>>& getEntries() override;
        const vector<Entry<KeyT, PayloadT>>& getEntries() const override;
        bool isLeafNode() const override;
};

template<typename KeyT, typename PayloadT>
class LeafNode: public BPlusNode<KeyT, PayloadT> {
    public:
        LeafNode();
        vector<Entry<KeyT, PayloadT>>& getEntries() override;
        const vector<Entry<KeyT, PayloadT>>& getEntries() const override;
        bool isLeafNode() const override;
};

#endif 
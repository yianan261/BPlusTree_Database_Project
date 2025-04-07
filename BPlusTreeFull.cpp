// B+ Tree with (key, value) pairs, insert/delete, split/merge support (skeleton with TODOs)
#include <iostream>
#include <vector>
#include <algorithm>
#include <map>
using namespace std;

const int ORDER = 3; // Max keys per node = ORDER - 1

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

class BPlusTree {
private:
    BPlusNode* root;

    void insertInternal(int key, string value, BPlusNode* node, BPlusNode*& newChild, int& newKey);
    void splitLeaf(BPlusNode* node, BPlusNode*& newChild, int& newKey);
    void splitInternal(BPlusNode* node, BPlusNode*& newChild, int& newKey);
    bool deleteEntry(BPlusNode* node, int key);
    void mergeLeaf(BPlusNode* node);
    void borrowLeaf(BPlusNode* node);
    void printTree(BPlusNode* node, int level);

public:
    BPlusTree() { root = new BPlusNode(true); }

    void insert(int key, string value);
    void remove(int key);
    void print();
    void printLeaves();
    string search(int key);
};

void BPlusTree::insert(int key, string value) {
    // TODO: Handle insertion. Call insertInternal and handle new root creation if split occurs.
}

void BPlusTree::insertInternal(int key, string value, BPlusNode* node, BPlusNode*& newChild, int& newKey) {
    // TODO: Recursively insert key-value into correct leaf.
    // If node is leaf, insert and split if necessary.
    // If internal, delegate to appropriate child and handle potential split from below.
}

void BPlusTree::splitLeaf(BPlusNode* node, BPlusNode*& newChild, int& newKey) {
    // TODO: Split a full leaf node into two and maintain linked list connections.
    // Assign newChild and newKey for parent node to handle.
    newChild = new BPlusNode(true);
    int midindex = ORDER / 2;

    newChild->entries.assign(node->entries.begin() + midindex, node->entries.end());
    newKey = newChild->entries.front().key;

    if(node -> next) {
        newChild -> next  = node -> next;
        node -> next -> prev = newChild;
    }
    newChild -> prev = node;
    node -> next = newChild;
}

void BPlusTree::splitInternal(BPlusNode* node, BPlusNode*& newChild, int& newKey) {
    // TODO: Split a full internal node and promote the middle key to the parent.
    // Assign newChild and newKey accordingly.
    newChild = new BPlusNode(false);
    int midindex = ORDER / 2;

    newKey = node->keys[midindex];
    newChild->keys.assign(node->keys.begin() + midindex + 1, node->keys.end());
    newChild->children.assign(node->children.begin() + midindex + 1, node->children.end());
    
    node->keys.resize(midindex);
    node->children.resize(midindex + 1);
}

void BPlusTree::remove(int key) {
    // TODO: Call deleteEntry. If root becomes empty, demote it to its only child.
}

bool BPlusTree::deleteEntry(BPlusNode* node, int key) {
    // TODO: Delete key from leaf or recurse into internal node.
    // Handle underflow in leaf by borrowing or merging.
    // Return true if deletion happened, false otherwise.
}

void BPlusTree::borrowLeaf(BPlusNode* node) {
    // TODO: Borrow entry from sibling leaf (either prev or next).
    // If borrowing is not possible, fallback to merging.
}

void BPlusTree::mergeLeaf(BPlusNode* node) {
    // TODO: Merge current leaf node with its sibling.
    // Update linked list pointers and delete the node.
}

string BPlusTree::search(int key) {
    // TODO: Traverse the tree to find the leaf node.
    // Then search through entries to find the value.
    return "";
}

void BPlusTree::print() {
    // TODO: Print the entire B+ tree structure for debugging.
    printTree(root, 0);
}

void BPlusTree::printTree(BPlusNode* node, int level) {
    // TODO: Recursively print node data and structure with indentation.
    if(!node) return;
    cout << string (level * 4, ' ') ;
    if (node -> isLeaf){
        cout << "Leaf: ";
        for (const auto& entry : node->entries) {
            cout << "(" << entry.key << ", " << entry.value << ") ";
        }
    }
    else{
        cout << "Internal: ";
        for (const auto& key : node->keys) {
            cout << key << " ";
        }
    }
    cout << endl;
    if (!(node -> isLeaf))
        for (auto child : node->children) {
            printTree(child, level + 1);
        }

}

void BPlusTree::printLeaves() {
    // TODO: Traverse to the leftmost leaf and print all entries left-to-right.
}

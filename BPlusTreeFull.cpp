// B+ Tree with (key, value) pairs, insert/delete, split/merge support (skeleton functions)
#include <iostream>
#include <vector>
#include <algorithm>
#include <map>
#include "BPlusNode.h"
#include "BPlusTree.h"

using namespace std;

const int ORDER = 3; // Max keys per node = ORDER - 1

BPlusTree::BPlusTree() {
    root = new LeafNode();
}

void BPlusTree::insert(int key, string value) {
    BPlusNode* newChild = nullptr;
    int newKey;
    insertInternal(key, value, root, newChild, newKey);
    if (newChild != nullptr) {
        InternalNode* newRoot = new InternalNode();
        newRoot->getKeys().push_back(newKey);
        newRoot->getChildren().push_back(root);
        newRoot->getChildren().push_back(newChild);
        root = newRoot;
    }
}

void BPlusTree::insertInternal(int key, string value, BPlusNode* node, BPlusNode*& newChild, int& newKey){
    if (node->isLeafNode()){
        auto& entries = static_cast<LeafNode*>(node)->getEntries();
        //lower_bound(start, end, value, comparator)
        auto insert_pos = lower_bound(entries.begin(), entries.end(), key, [](const Entry& a, int b){
            return a.key < b;
        });
        entries.insert(insert_pos, {key, value});
        if(entries.size() >= ORDER){
            splitLeaf(node, newChild, newKey);
        }
        return;
    }
    auto& keys = node->getKeys();
    auto& children = node->getChildren();
    int insert_pos = upper_bound(keys.begin(), keys.end(), key) - keys.begin();
    BPlusNode* child = children[insert_pos];
    BPlusNode* tempChild = nullptr;
    int tempKey;
    insertInternal(key, value, child, tempChild, tempKey);
    if(tempChild){
        auto* internal = static_cast<InternalNode*>(node);
        internal->getKeys().insert(keys.begin() + insert_pos, tempKey);
        internal->getChildren().insert(children.begin()+insert_pos+1, tempChild);
        if (internal->getKeys().size() >= ORDER){
            splitInternal(node, newChild, newKey);
        }
    }

}

void BPlusTree::splitLeaf(BPlusNode* node, BPlusNode*& newChild, int& newKey) {
    // Split a full leaf node into two and maintain linked list connections.
    // Assign newChild and newKey for parent node to handle.
    LeafNode* newLeaf = new LeafNode();
    auto& entries = static_cast<LeafNode*>(node)->getEntries();
    auto& newEntries = newLeaf->getEntries();

    int mid = ORDER/2;
    newEntries.assign(entries.begin()+mid, entries.end());
    entries.resize(mid);

    newKey = newEntries.front().key; // promote to parent

    newLeaf->setNext(node->getNext());
    if(node->getNext()) node->getNext()->setPrev(newLeaf);
    newLeaf->setPrev(node);
    node->setNext(newLeaf);

    newChild = newLeaf;
}

void BPlusTree::splitInternal(BPlusNode* node, BPlusNode*& newChild, int& newKey) {
    // TODO: Split a full internal node and promote the middle key to the parent.
    // Assign newChild and newKey accordingly.
    InternalNode* newInternal = new InternalNode();
    auto& keys = node->getKeys();
    auto& children = node->getChildren();
    auto& newKeys = newInternal->getKeys();
    auto& newChildren = newInternal->getChildren();

    int mid = ORDER/2;
    newKey = keys[mid];
    
    newKeys.assign(keys.begin() + mid + 1, keys.end());
    newChildren.assign(children.begin() + mid + 1, children.end());

    keys.resize(mid);
    children.resize(mid+1);

    newChild = newInternal;
}

void BPlusTree::remove(int key) {
    // TODO: Call deleteEntry. If root becomes empty, demote it to its only child.
    // Zirui
}

bool BPlusTree::deleteEntry(BPlusNode* node, int key) {
    // TODO: Delete key from leaf or recurse into internal node.
    // Handle underflow in leaf by borrowing or merging.
    // Return true if deletion happened, false otherwise.
    //Zirui
    return 0;
}

void BPlusTree::borrowLeaf(BPlusNode* node) {
    // TODO: Borrow entry from sibling leaf (either prev or next).
    // If borrowing is not possible, fallback to merging.
    //Yian
    
    return;
}

void BPlusTree::mergeLeaf(BPlusNode* node) {
    // TODO: Merge current leaf node with its sibling.
    // Update linked list pointers and delete the node.
    //Yian
    return;
}

string BPlusTree::search(int key) {
    // TODO: Traverse the tree to find the leaf node.
    // Then search through entries to find the value.
    // Yian
    return "";
}

void BPlusTree::print() {
    // TODO: Print the entire B+ tree structure for debugging.
    printTree(root, 0);
}

void BPlusTree::printTree(BPlusNode* node, int level) {
    // TODO: Recursively print node data and structure with indentation.
    if (!node) return;
    cout << string(level * 4, ' ');
    if (node->isLeafNode()) {
        auto& entries = static_cast<LeafNode*>(node)->getEntries();
        cout << "Leaf: ";
        for (const auto& e : entries) {
            cout << "(" << e.key << ", " << e.value << ") ";
        }
    } else {
        cout << "Internal: ";
        for (int k : node->getKeys()) {
            cout << k << " ";
        }
    }
    cout << endl;
    if (!node->isLeafNode()) {
        for (BPlusNode* child : node->getChildren()) {
            printTree(child, level + 1);
        }
    }

}

void BPlusTree::printLeaves() {
    // TODO: Traverse to the leftmost leaf and print all entries left-to-right.
    return;
}

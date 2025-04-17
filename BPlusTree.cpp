#include "BPlusTree.h"
#include <iostream>
#include <algorithm>
using namespace std;

const int ORDER = 3;
const int MIN_ALLOWED = (ORDER - 1) / 2;

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
    if (deleteEntry(root, key) && !root->isLeafNode() && root->getKeys().empty()) {
        root = root->getChildren().front();
    }

}

bool BPlusTree::deleteEntry(BPlusNode* node, int key) {
    // TODO: Delete key from leaf or recurse into internal node.
    // Handle underflow in leaf by borrowing or merging.
    // Return true if deletion happened, false otherwise.
    //Zirui
    if (node->isLeafNode()) {
        auto& entries = static_cast<LeafNode*>(node)->getEntries();
        auto it = find_if(entries.begin(), entries.end(), [key](const Entry& e) { return e.key == key; });
        if (it != entries.end()) {
            entries.erase(it);
            if (entries.size() < MIN_ALLOWED) {
                borrowLeaf(node);
            }
            return true;
        }
        return false;
    }

    auto& keys = node->getKeys();
    auto& children = node->getChildren();
    int childIndex = upper_bound(keys.begin(), keys.end(), key) - keys.begin();
    return deleteEntry(children[childIndex], key);
}

void BPlusTree::borrowLeaf(BPlusNode* node) {
    // Borrow entry from sibling leaf (either prev or next).
    // If borrowing is not possible, fallback to merging.
    auto* leaf = static_cast<LeafNode*>(node);
    auto* prev = node->getPrev(); //BPlusNode* base pointer
    auto* next = node->getNext();
    // check left sibling to borrow, convert to LeafNode()
    if (prev && static_cast<LeafNode*>(prev)->getEntries().size() > MIN_ALLOWED){
        auto& prevEntries = static_cast<LeafNode*>(prev)->getEntries();
        auto& entries = leaf->getEntries();
        entries.insert(entries.begin(), prevEntries.back()); // insert to front
        prevEntries.pop_back();
        return;
    } // check right sibling to borrow
    else if(next && static_cast<LeafNode*>(next)->getEntries().size() > MIN_ALLOWED){
        auto& nextEntries = static_cast<LeafNode*>(next)->getEntries();
        auto& entries = leaf->getEntries();
        entries.push_back(nextEntries.front()); // insert to back
        nextEntries.erase(nextEntries.begin());
        return;
    }
    mergeLeaf(node); // if cannot borrow, merge current leaf with prev or next sibling
}

void BPlusTree::mergeLeaf(BPlusNode* node) {
    // Merge current leaf node with its sibling.
    // Update linked list pointers and delete the node.
    auto* leaf = static_cast<LeafNode*>(node);
    auto* prev = leaf->getPrev();
    auto* next = leaf->getNext();
    // check previous or next leaf to merge
    if (prev) {
        auto& prevEntries = static_cast<LeafNode*>(prev)->getEntries();
        auto& entries = leaf->getEntries();
        prevEntries.insert(prevEntries.end(), entries.begin(), entries.end());
        prev->setNext(next);
        if (next) next->setPrev(prev);
        delete leaf;
    } 
    else if (next) {
        auto& nextEntries = static_cast<LeafNode*>(next)->getEntries();
        auto& entries = leaf->getEntries();
        nextEntries.insert(nextEntries.begin(), entries.begin(), entries.end());
        next->setPrev(nullptr);
        delete leaf;
    }
    
}

string BPlusTree::search(int key) {
    // Traverse the tree to find the leaf node.
    // Then search through entries to find the value.
    BPlusNode* node = root;
    while (!node->isLeafNode()) {
        auto& keys = node->getKeys();
        auto& children = node->getChildren();
        int idx = upper_bound(keys.begin(), keys.end(), key) - keys.begin();
        node = children[idx];
    }

    auto& entries = static_cast<LeafNode*>(node)->getEntries();
    for (const auto& entry : entries) {
        if (entry.key == key) return entry.value;
    }
    return ""; // if no entry found
}

void BPlusTree::print() {
    // TODO: Print the entire B+ tree structure for debugging.
    printTree(root, 0);
}

void BPlusTree::printTree(BPlusNode* node, int level) {
    // Recursively print node data and structure with indentation.
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
    //Traverse to the leftmost leaf and print all entries left-to-right.
    BPlusNode* node = root;
    while (!node->isLeafNode()) {
        node = node->getChildren().front();
    }
    while (node) {
        auto& entries = static_cast<LeafNode*>(node)->getEntries();
        for (const auto& e : entries) {
            cout << "(" << e.key << ", " << e.value << ") ";
        }
        node = node->getNext();
    }
    cout << endl;
}
